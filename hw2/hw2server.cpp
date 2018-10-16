#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <algorithm>

#define MAX_CLIENTS 5
#define MAXLINE 1024


std::string getRandomWord(std::vector<std::string> &wordsList, int maxSize) {
	int randomIndex = rand() % wordsList.size();

	std::string secretword = wordsList[randomIndex];
	//remove the newline
	secretword.erase(std::remove(secretword.begin(), secretword.end(), '\n'), secretword.end());
	secretword.erase(std::remove(secretword.begin(), secretword.end(), ' '), secretword.end());
	secretword.erase(std::remove(secretword.begin(), secretword.end(), '\r'), secretword.end());

	if (secretword.size() > maxSize) {
		return getRandomWord(wordsList, maxSize);
	}else{
		return secretword;
	}
} 


int main(int argc, char* argv[]) {

	if (argc != 3) {
		std::cout << "Wrong number of arguments. Please run the program like this: ./hw2server.out [ipAddress] [maxWordSize]\n";
		return -1;
	}

	int maxfds;
	int numclients = 0;
	std::string secretword;
	int clifds[MAX_CLIENTS];
	std::vector<std::string> clinames(MAX_CLIENTS,"");
	struct sockaddr_in servaddr;
	socklen_t sockaddr_len = sizeof(servaddr);
	std::string username = "Choose a unique username: ";


	//initialize array of client fds 
	//-1 represents an empty spot
	for(int i=0; i<MAX_CLIENTS; i++) {
		clifds[i] = -1;
	}

	//create the server address
	memset(&servaddr, 0, sockaddr_len);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(0);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//create a vector of all words in the dictionary
	std::string dictionary = argv[1];
	int maxSize = atoi(argv[2]);

	std::vector<std::string> wordsList;

	std::ifstream input(dictionary.c_str());

	if (!input) {
		std::cout << "Cannot open input file" << std::endl;
	}

	for (std::string line; getline(input, line);) {
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		wordsList.push_back(line);
	}

	//open the socket 
	int lsock;
	if ((lsock = socket(PF_INET,SOCK_STREAM, 0)) < 0) {
		perror("could not create listen socket\n");
		return 1;
	}

	//bind the socket
	if ((bind(lsock, (struct sockaddr *)&servaddr, sockaddr_len)) < 0) {
		perror("failed binding socket\n");
		return 1;
	}

	//open socket for listening
	if (listen(lsock, 10) < 0) {
		perror("failed to open socket for listening\n");
		return 1;
	}

	//print out new port for the socket
	getsockname(lsock, (struct sockaddr *)&servaddr, &sockaddr_len);
	printf("Port: %d\n",ntohs(servaddr.sin_port));

	//choose a random word from the dictionary
	srand(servaddr.sin_port);

	secretword = getRandomWord(wordsList, maxSize);
	
	std::cout << secretword << std::endl;

	std::vector<char> wordInfo;

	int wordlen =0;
	//find the length of the word
	while(secretword.c_str()[wordlen] !='\0')
	{
		wordlen++;
	}
	
	//create a vector of the secret word
	for (int i = 0; i < wordlen; i++) {
		wordInfo.push_back(secretword[i]);
	}

	maxfds = lsock;
	struct sockaddr_in cliaddr;
	socklen_t clilen = 0;

	fd_set rset;
	while(1) {
		FD_ZERO(&rset);

		//if we can take more clients, add listen socket to rset
		if (numclients < MAX_CLIENTS) {
			FD_SET(lsock, &rset);
			maxfds = lsock;
		}

		//add all connected clients to rset
		for(int i=0; i < MAX_CLIENTS; i++) {
			if (clifds[i] != -1) {
				FD_SET(clifds[i], &rset);
				maxfds = std::max(maxfds, clifds[i]);
			}
		}

		maxfds++;
		select(maxfds, &rset, NULL, NULL, NULL); //select call to find changed fds

		//if listen fd has changed, a new client is trying to connect
		if (numclients < MAX_CLIENTS && FD_ISSET(lsock, &rset)) {
			int clisock;
			if ((clisock = accept(lsock, (struct sockaddr *)&cliaddr, &clilen )) < 0) {
				perror("unable to open socket to accept");
				return 1;
			}

			//find an empty spot in array to store client fd
			int cli_index;
			for (int i=0; i<MAX_CLIENTS; i++) {
				if (clifds[i] == -1) {
					clifds[i] = clisock;
					cli_index = i;
					break;
				}
			}


			//the new client must choose a new username
			//if they choose a username that is already taken, resend 
			//instructions to choose a username
			write(clisock, username.c_str(), username.length());
			int n;
			int namechosen = 0;
			char buffer[MAXLINE];
			std::string name;
			while(!namechosen) {
				int nametaken = 0;
				n = read(clifds[cli_index], buffer, MAXLINE);
				name.assign(buffer, n);
				name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
				name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
				name.erase(std::remove(name.begin(), name.end(), '\r'), name.end());
				for (int j=0; j<MAX_CLIENTS; j++) {
					if (name == clinames[j]) {
						nametaken = 1;
						break;
					}
				}
				if (nametaken) {
					write(clisock, username.c_str(), username.length());
				}
				else {
					clinames[cli_index] = name;
					namechosen = 1;
				}
			}

			//send amount of letters to the client.
			std::string instructions = "There are " + std::to_string(numclients) + " clients currently playing.\n";
			instructions = instructions + "The secret word has " + std::to_string(wordlen) + " letters.\n";
			write(clisock, instructions.c_str(), instructions.length());
			numclients++;
		}

		//iterate through all client fds to see which have changed
		for (int i=0; i<MAX_CLIENTS; i++) {
			if (clifds[i] != -1)  {
				if (FD_ISSET(clifds[i], &rset)) { //if a given fd has changed
					char buffer[MAXLINE];
					int n;

					//If a client disconnects, remove thier info and username, close socket.
					if ((n =read(clifds[i], buffer, MAXLINE)) == 0) {
						close(clifds[i]);
						clifds[i] = -1;
						clinames[i] = "";
						numclients--;
						continue;
					}

					//Client guess of a word
					std::string guess;
					guess.assign(buffer,n);
					std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);

					//remove the newline
					guess.erase(std::remove(guess.begin(), guess.end(), '\n'), guess.end());

					//If a client guesses a word of the wrong length, send error message
					//but do not disconnect the client
					if (guess.length() != secretword.length()) {
						std::string errmesg;
						errmesg = "You must guess a word with " + std::to_string(wordlen) + " letters\n";
						write(clifds[i],  errmesg.c_str(), errmesg.length());
						continue;
					}
					//Analyze the client's guess and output its info.


					int numCorrect = 0;
					int numPlaced = 0;

					//create a temp char vector from the randomly picked word
					std::vector<char> tempWord = wordInfo;


					//get num correct
					for (int j = 0; j < guess.size(); j++) {
						std::vector<char>::iterator itr = std::find(tempWord.begin(), tempWord.end(), guess[j]);

						if (itr != tempWord.end()) {
							int loc = std::distance(tempWord.begin(), itr);
							tempWord.erase(tempWord.begin() + loc);
							numCorrect++;
						}
						//else the element wasn't found
					}


					//Count all letters that are correctly placed
					//get the num placed
					for (int j= 0; j < guess.size(); j++) {
						if (guess[j] == secretword[j]) {
							numPlaced++;
						}
					}

					//A client wins the game!
					//Disconnect all clients and choose a new random word
					if (guess == secretword) {
						std::string winmesg = clinames[i] + " has correclty guessed the word ";
						winmesg = winmesg + secretword + ".\n";
						for (int j=0; j<MAX_CLIENTS; j++) {
							if (clifds[j] != -1) {
								write(clifds[j], winmesg.c_str(), winmesg.length());
								close(clifds[j]);
								clifds[j] = -1;
								clinames[j] = "";
								numclients = 0;
							}
						}
						secretword = getRandomWord(wordsList, maxSize);

						std::cout << secretword << std::endl;
						wordlen = secretword.length();

						wordInfo.clear();
						for (int k = 0; k < wordlen; k++) {
							wordInfo.push_back(secretword[k]);
						}

						
						break;
					}

					//send information to all clients when one client makes a guess.
					std::string guessinfo;
					guessinfo = clinames[i] + " guessed " + guess +  ": " + std::to_string(numCorrect);
					guessinfo = guessinfo + " letter(s) were correct and " + std::to_string(numPlaced);
					guessinfo = guessinfo + " letter(s) were correctly placed.\n";

					for (int j=0; j<MAX_CLIENTS; j++) {
						if (clifds[j] != -1)
							write(clifds[j], guessinfo.c_str(), guessinfo.length());
					}
				}
			}
		}

	}



}