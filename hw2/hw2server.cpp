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
	memset(&servaddr, 0, sizeof(servaddr));
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

	srand(servaddr.sin_port);
	int randomIndex = rand() % wordsList.size();

	secretword = wordsList[randomIndex];


	printf("Port: %d\n",servaddr.sin_port);

	std::vector<char> wordInfo;
	int wordlen = secretword.length();
	for (int i = 0; i < wordlen; i++) {
		wordInfo.push_back(tolower(secretword[i]));
	}

	int lsock;
	if ((lsock = socket(PF_INET,SOCK_STREAM, 0)) < 0) {
		perror("could not create listen socket\n");
		return 1;
	}

	if ((bind(lsock, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
		perror("failed binding socket\n");
		return 1;
	}

	if (listen(lsock, 10) < 0) {
		perror("failed to open socket for listening\n");
		return 1;
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
				}
			}

			std::string username = "Choose a unique username: ";
			int choosingname = 1;
			int nametaken = 0;
			int n;
			while(choosingname) {
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
					choosingname = 0;
				}
			}
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
						continue;
					}

					//Client guess of a word
					std::string guess;
					guess.assign(buffer,n);

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
					for (int i = 0; i < guess.size(); i++) {
						std::vector<char>::iterator itr = std::find(tempWord.begin(), tempWord.end(), tolower(guess[i]));

						if (itr != tempWord.end()) {
							int loc = std::distance(tempWord.begin(), itr);
							tempWord.erase(tempWord.begin() + loc);
							numCorrect++;
						}
						//else the element wasn't found
					}


					//Count all letters that are correctly placed
					//get the num placed
					for (int i = 0; i < guess.size(); i++) {
						if (tolower(guess[i]) == tolower(secretword[i])) {
							numPlaced++;
						}
					}


				}
			}
		}

	}



}