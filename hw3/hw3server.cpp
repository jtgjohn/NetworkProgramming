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
#include <regex>

#define MAXLINE 1024

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
	std::unordered_map<std::string, std::unordered_set<std::string> > channels;
	std::unordered_map<std::string, std::unordered_set<std::string> > user_channels;
	int numclients = 0;
	int password_set = 0;
	std::string password;
	std::string message;
	int maxfds;
  std::regex pwdAllowed("[a-zA-Z][_0-9a-zA-Z]*");
  std::regex usrAllowed("[a-zA-Z][_0-9a-zA-Z]*");
  std::unordered_set<std::string> operators;

  if (argc > 1) {
    std::string passInput = argv[1];
    if (passInput.find("--opt-pass=") == 0) {
      password = passInput.substr(11, passInput.size());

      //regex check the password
      if ((regex_match(password, pwdAllowed)) && password.length() < 21) {
        password_set = 1;
      }
    }
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
		for(int i=0; i < clifds.size(); i++) {
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
			numclients++;


		}


		for (int i=0; i<clifds.size(); i++) {

			if(FD_ISSET(clifds[i], &rset)) {

				char buffer[MAXLINE];
				int n;

				//If a client disconnects, remove thier info and username, close socket.
				if ((n =read(clifds[i], buffer, MAXLINE)) == 0) {
					close(clifds[i]);
					clifds.erase(clifds.begin() + i);
					usernames.erase(usernames.begin() + i);
					numclients--;
					i--;
					continue;
				}

				//Client guess of a word
				std::string input;
				input.assign(buffer,n);
				std::vector<std::string> command_list = parser(input);
				std::string command = command_list[0];

				//if no command has been entered yet
				if(usernames[i] == "") {
					if (command == "USER") {
						int invalid_username = 0;

						if (command_list.size() < 2) {
							invalid_username = 1;
						}

						for (int j=0; j<usernames.size(); j++) {
							if (usernames[j] == command_list[1]) {
								invalid_username = 1;
								break;
							}
						}

            if (!(regex_match(command_list[1], usrAllowed)) || command_list[1].length() > 20) {
              invalid_username = 1;
            }

						if (invalid_username) {
							message = "Invalid username.\n";
							write(clifds[i], message.c_str(), message.length());
							continue;
						}

						usernames[i] = command_list[1];
						message = "Welcome, " + command_list[1] + ".\n";

					} else {
						message = "Invalid command, please identify yourself with USER.\n";
						write(clifds[i], message.c_str(), message.length());

						close(clifds[i]);
						clifds.erase(clifds.begin() + i);
						usernames.erase(usernames.begin() + i);
						numclients--;
						i--;


					}
				} else if(command == "USER") {
					message = "You cannot change your username after it is set.\n";
					write(clifds[i], message.c_str(), message.length());

				} else if (command == "LIST") {
					int list_channels = 1;
					std::string channel;

					if (command_list.size() > 1) {
            //iterate through channels and compare each to command_list[1], if equal set channel
            std::unordered_map<std::string, std::unordered_set<std::string> >::iterator it = channels.begin();
						while (it != channels.end()) {
              if (it->first == command_list[i]) {
                channel = command_list[i];
                list_channels = 0;
              }
              it++;
            }
					}

					if (list_channels) {
            std::unordered_map<std::string, std::unordered_set<std::string> >::iterator it = channels.begin();
						message = "There are currently " + std::to_string(channels.size()) + " channels.\n";
						while (it != channels.end()) {
							message += "* " + it->first + "\n";
              it++;
						}
					} else {
						message = "There are currently " + std::to_string(channels[command_list[1]].size()) + " members.\n" + channel + "members:";
						std::unordered_set<std::string>::iterator itr = channels[command_list[1]].begin();
						for(; itr != channels[command_list[1]].end(); ++itr) {
							message += " " + *itr;
						}
						message += "\n";
					}

					write(clifds[i], message.c_str(), message.length());

				} else if (command == "JOIN") {
					if (command_list.size() < 2) {
						message = "Invalid JOIN command.\n";
					} else { //Valid join command
						if (channels.count(command_list[1]) == 0) { //channel doesnt exist, create it
							std::unordered_set<std::string> emptyset;
							emptyset.insert(command_list[1]);
							channels[command_list[1]] = emptyset;
						} else {
							std::unordered_set<std::string>::iterator itr = channels[command_list[1]].begin();
							message = command_list[1] + "> "  +usernames[i] + " joined the channel.\n";
							for (; itr != channels[command_list[1]].end(); ++itr) {
								for (int j=0; j<usernames.size(); j++) {
									if (usernames[j] == *itr) {
										write(clifds[j], message.c_str(), message.length());
										break;
									}
								}
							}
							channels[command_list[1]].insert(usernames[i]);
						}
						message = "Joined channel " + command_list[1] + ".\n";
					}

					write(clifds[i], message.c_str(), message.length());

				} else if (command == "PART") {
					if (command_list.size() > 1) {
						if (channels.count(command_list[1]) == 0) { //channel does not exist
							message = "You are not currently in channel " + command_list[1] + ".\n";
						} else { //channel exists

							int in_channel = channels[command_list[1]].erase(usernames[i]);

							if (in_channel) {
								std::unordered_set<std::string>::iterator itr = channels[command_list[1]].begin();
								message = command_list[1] + "> " + usernames[i] + " left the channel.\n";
								for (;itr != channels[command_list[1]].end(); ++itr) {
									for (int j=0; j<usernames.size(); j++) {
										if (usernames[j] == *itr) {
											write(clifds[j], message.c_str(), message.length());
											break;
										}
									}
 								}
							} else {
								message = "You are not currently in " + command_list[1] + ".\n";
							}
						}
						write(clifds[i], message.c_str(), message.length());
					} else { //remove them from all channels
						std::unordered_map<std::string, std::unordered_set<std::string> >::iterator itr = channels.begin();
						for(;itr != channels.end(); ++itr) {
							if ((itr->second).erase(usernames[i]) == 1) {
								std::unordered_set<std::string>::iterator itr2 = (itr->second).begin();
								message = itr->first + "> " + usernames[i] + " left the channel.\n";
								for (;itr2 != (itr->second).end(); ++itr2) {
									for (int j=0; j<usernames.size(); j++) {
										if (usernames[j] == *itr2) {
											write(clifds[j], message.c_str(), message.length());
											break;
										}
									}
								}
								write(clifds[i], message.c_str(), message.length());
							}
						}
					}

				} else if (command == "OPERATOR") {
					if (password_set) {
						if (command_list.size() > 1 && command_list[1] == password) {
							operators.insert(usernames[i]);
							message = "OPERATOR status bestowed.\n";
						} else {
							message = "Invalid OPERATOR command.\n";
						}
					} else {
						message = "No operators allowed.\n";
					}

					write(clifds[i], message.c_str(), message.length());

				} else if (command == "KICK") {
					if (command_list.size() < 3) {
						message = "invalid KICK command.\n";
					} else {
						if (operators.count(usernames[i]) == 1) {
							if (channels.count(command_list[1]) == 1) { //if it is a valid channel
								//if the user is actually in the specified channel
								if (channels[command_list[1]].count(command_list[2]) == 1) {
									message = command_list[1] + "> " + command_list[2] + " has been kicked from the channel.\n";
									std::unordered_set<std::string>::iterator itr = channels[command_list[1]].begin();
									for (; itr != channels[command_list[1]].end(); ++itr) {
										for (int j=0; j<usernames.size(); j++) {
											if (*itr == usernames[j] && usernames[i] != usernames[j]) {
												write(clifds[j], message.c_str(), message.length());
												break;
											}
										}
									}
									channels[command_list[1]].erase(command_list[2]);
								}
							}
						} else { // current user is not an operator
							message = "You must be an operator to kick another user.\n";
						}
					}

					write(clifds[i], message.c_str(), message.length());

				} else if (command == "PRIVMSG") {

				} else if (command == "QUIT") {

				} else { //INVALID COMMAND

				}


			}
		}

	}



}
