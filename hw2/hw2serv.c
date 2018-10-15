#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

#define MAX_CLIENTS 5
#define MAXLINE 1024


int main() {

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

	printf("Port: %d\n",servaddr.sin_port);

	int lsock;
	if ((lscok = socket(PF_INET,SOCK_STREAM, 0)) < 0) {
		perror("could not create listen socket\n");
		return 1;
	}

	if ((bind(lsock, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
		perror("failed binding socket\n");
		return 1;
	}

	if (listen(lsock, 1) < 0) {
		perror("failed to open socket for listening\n");
		return 1;
	}
	maxfds = lsock;
	struct sockaddr_in cliaddr;
	int clilen = 0;

	fd_set rset;
	while(1) {
		FD_ZERO(&rset);

		if (numclients < MAX_CLIENTS) {
			FD_SET(lsock, &rset);
			maxfds = lsock;
		}

		for(int i=0; i < MAX_CLIENTS; i++) {
			if (clifd[i] != -1) {
				FD_SET(clifd[i], &rset);
				maxfds = max(maxfds, clifd[i]);
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
				if (clifd[i] == -1) {
					clifd[i] = clisock;
					cli_index = i;
				}
			}

			std::string username = "Choose a unique username: ";
			int choosingname = 1;
			int nametaken = 0;
			while(choosingname) {
				char buffer[MAXLINE];
				read(clisock, buffer, MAXLINE);
				for (int i=0; i<MAX_CLIENTS; i++) {
					if (clinames[i] == buffer) {
						nametaken = 1;
						break;
					}
				}
				if (!nametaken) {
					clinames[cli_index] = str(buffer);
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
					std::string guess = str(buffer);

					//If a client guesses a word of the wrong length, send error message
					//but do not disconnect the client
					if (guess.length() != secretword.length()) {
						std::string errmesg;
						errmesg = "You must guess a word with " + str(secretword.length()) + " characters\n";
						write(clifds[i],  errmesg.c_str(), errmesg.length());
						continue;
					}
					//Analyze the client's guess and output its info.
					
				}
			}
		}

	}


}