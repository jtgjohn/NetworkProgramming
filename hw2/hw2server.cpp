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


int main(int argc, char* argv[]) {

	int maxfds;
	int numclients = 0;
	std::string secretword;
	int clifds[MAX_CLIENTS];
	std::vector<std::string> clinames(MAX_CLIENTS,"");
	struct sockaddr_in servaddr;
	socklen_t sockaddr_len = sizeof(servaddr);
	std::string username = "Choose a unique username: ";


	for(int i=0; i<MAX_CLIENTS; i++) {
		clifds[i] = -1;
	}

	memset(&servaddr, 0, sockaddr_len);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(0);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


	std::string dictionary = argv[1];
	std::vector<std::string> wordsList;

	std::ifstream input(dictionary.c_str());

	if (!input) {
		std::cout << "Cannot open input file" << std::endl;
	}

	for (std::string line; getline(input, line);) {
		wordsList.push_back(line);
	}

	int lsock;
	if ((lsock = socket(PF_INET,SOCK_STREAM, 0)) < 0) {
		perror("could not create listen socket\n");
		return 1;
	}

	if ((bind(lsock, (struct sockaddr *)&servaddr, sockaddr_len)) < 0) {
		perror("failed binding socket\n");
		return 1;
	}

	if (listen(lsock, 10) < 0) {
		perror("failed to open socket for listening\n");
		return 1;
	}

	getsockname(lsock, (struct sockaddr *)&servaddr, &sockaddr_len);
	printf("Port: %d\n",ntohs(servaddr.sin_port));

	srand(servaddr.sin_port);
	int randomIndex = rand() % wordsList.size();

	secretword = wordsList[randomIndex];
	//remove the newline
	secretword.erase(std::remove(secretword.begin(), secretword.end(), '\n'), secretword.end());
	secretword.erase(std::remove(secretword.begin(), secretword.end(), ' '), secretword.end());

	if (!secretword.empty() && secretword[secretword.length()-1] == '\n') {
	    secretword.erase(secretword.length()-1);
	}
	if (!secretword.empty() && secretword[secretword.length()-1] == ' ') {
	  secretword.erase(secretword.length()-1);
	}
	if (!secretword.empty() && secretword[secretword.length()-1] == '\r') {
	  secretword.erase(secretword.length()-1);
	}
	std::cout << secretword << std::endl;

	std::vector<char> wordInfo;

	int wordlen =0;

	while(secretword.c_str()[wordlen] !='\0')
	{
		wordlen++;
	}
	

	//int wordlen = secretword.length();
	for (int i = 0; i < wordlen; i++) {
		wordInfo.push_back(tolower(secretword[i]));
	}

	maxfds = lsock;
	struct sockaddr_in cliaddr;
	socklen_t clilen = 0;

	fd_set rset;
	while(1) {
		FD_ZERO(&rset);

		if (numclients < MAX_CLIENTS) {
			FD_SET(lsock, &rset);
			maxfds = lsock;
		}

		for(int i=0; i < MAX_CLIENTS; i++) {
			if (clifds[i] != -1) {
				FD_SET(clifds[i], &rset);
				maxfds = std::max(maxfds, clifds[i]);
			}
		}

		maxfds++;
		select(maxfds, &rset, NULL, NULL, NULL);

		if (numclients < MAX_CLIENTS && FD_ISSET(lsock, &rset)) {
			int clisock;
			if ((clisock = accept(lsock, (struct sockaddr *)&cliaddr, &clilen )) < 0) {
				perror("unable to open socket to accept");
				return 1;
			}

			int cli_index;
			for (int i=0; i<MAX_CLIENTS; i++) {
				if (clifds[i] == -1) {
					clifds[i] = clisock;
					cli_index = i;
					break;
				}
			}


			int choosingname = 1;
			int nametaken = 0;
			int n;
			while(choosingname) {
				write(clisock, username.c_str(), username.length());
				char buffer[MAXLINE];
				n = read(clisock, buffer, MAXLINE);
				for (int i=0; i<MAX_CLIENTS; i++) {
					if (clinames[i] == buffer) {
						nametaken = 1;
						break;
					}
				}
				if (!nametaken) {
					clinames[cli_index].assign(buffer,n);
						clinames[cli_index].erase(std::remove(clinames[cli_index].begin(), clinames[cli_index].end(), '\n'), clinames[cli_index].end());
						clinames[cli_index].erase(std::remove(clinames[cli_index].begin(), clinames[cli_index].end(), ' '), clinames[cli_index].end());
						clinames[cli_index].erase(std::remove(clinames[cli_index].begin(), clinames[cli_index].end(), '\r'), clinames[cli_index].end());
					choosingname = 0;
				}
			}
			numclients++;
		}

		for (int i=0; i<MAX_CLIENTS; i++) {
			if (clifds[i] != -1)  {
				if (FD_ISSET(clifds[i], &rset)) {
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

					//remove the newline
					guess.erase(std::remove(guess.begin(), guess.end(), '\n'), guess.end());

					//If a client guesses a word of the wrong length, send error message
					//but do not disconnect the client
					if (guess.length() != secretword.length()) {
						std::string errmesg;
						errmesg = "You must guess a word with " + std::to_string(wordlen) + " characters\n";
						write(clifds[i],  errmesg.c_str(), errmesg.length());
						continue;
					}
					//Analyze the client's guess and output its info.


					int numCorrect = 0;
					int numPlaced = 0;

					//create a char vector from the chosen word
					std::vector<char> wordInfo;

					std::vector<char> tempWord = wordInfo;


					//get num correct
					for (int j = 0; j < guess.size(); j++) {
						std::vector<char>::iterator itr = std::find(tempWord.begin(), tempWord.end(), tolower(guess[j]));

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
						if (tolower(guess[j]) == tolower(secretword[j])) {
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
						randomIndex = rand() % wordsList.size();
						secretword = wordsList[randomIndex];
						//remove the newline
						secretword.erase(std::remove(secretword.begin(), secretword.end(), '\n'), secretword.end());
						secretword.erase(std::remove(secretword.begin(), secretword.end(), ' '), secretword.end());
						secretword.erase(std::remove(secretword.begin(), secretword.end(), '\r'), secretword.end());
						std::cout << secretword << std::endl;
						break;
					}

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