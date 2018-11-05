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
#include <unordered_map>
#include <unordered_set>

std::vector<std::string> parser(std::string toParse) {

  std::vector<std::string> parsed;

  std::string toAdd = "";

  for (int i = 0; i < toParse.size(); i++) {
    if (!(isspace(toParse.at(i)))) {
      toAdd += toParse.at(i);
    }else{
      parsed.push_back(toAdd);
      toAdd = "";
    }
  }
  parsed.push_back(toAdd);

  //clease the vector
  for (int i = 0; i < parsed.size(); i++) {
    if (parsed[i] == "") {
      parsed.erase(parsed.begin() + i);
      i--;
    }
  }

  return parsed;

}



int main(int argc, char* argv[]) {
	struct sockaddr_in servaddr;
	socklen_t sockaddr_len = sizeof(servaddr);
	std::vector<int> clifds;
	std::vector<std::string> usernames;
	std::unordered_map<std::string, std::vector<std::string> > channels;
	std::unordered_map<std::string, std::vector<std::string> > user_channels;
	int numclients = 0;
	int password_set = 0;
	std::string password;

	if (argc > 1) {
		password = argv[1]; //change this to take out flag
		password_set = 1;
	}






	//create the server address
	memset(&servaddr, 0, sockaddr_len);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(0);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


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

	maxfds = lsock;
	struct sockaddr_in cliaddr;
	socklen_t clilen = 0;

	fd_set rset;
	while(1) {
		FD_ZERO(&rset);

		//if we can take more clients, add listen socket to rset
		if (numclients < FD_SETSIZE) {
			FD_SET(lsock, &rset);
			maxfds = lsock;
		}

		//add all connected clients to rset
		for(int i=0; i < clifds.length(); i++) {
			FD_SET(clifds[i], &rset);
			maxfds = std::max(maxfds, clifds[i]);
		}

		maxfds++;
		select(maxfds, &rset, NULL, NULL, NULL); //select call to find changed fds

		//if listen fd has changed, a new client is trying to connect
		if (numclients < FD_SETSIZE && FD_ISSET(lsock, &rset)) {
			int clisock;
			if ((clisock = accept(lsock, (struct sockaddr *)&cliaddr, &clilen )) < 0) {
				perror("unable to open socket to accept");
				return 1;
			}

			//add the new client
			clifds.push_back(clisock);
			usernames.push_back(""); //placeholder while we wait for username to be selected


		}


		for (int i=0; i<clifds.length(); i++) {

			if(FD_ISSET(clifds[i])) {

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
				std::string input;
				input.assign(buffer,n);

				//if no command has been entered yet
				if(usernames[i] == "") {
					if (command == "USER") {

					} else {

					}
				} else if(command == "USER") {
					std::string message = "You cannot change your username.\n";
					write(clifds[i], message.c_str(), message.length());

				} else if (command == "LIST") {

				} else if (command == "JOIN") {

				} else if (command == "PART") {

				} else if (command == "OPERATOR") {

				} else if (command == "KICK") {

				} else if (command == "PRIVMSG") {

				} else if (command == "QUIT") {

				} else { //INVALID COMMAND

				}


			}
		}

	}



}