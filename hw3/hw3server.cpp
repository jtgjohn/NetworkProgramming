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
    if (parsed.size() == 2 && parsed[0] == "PRIVMSG") {
      toAdd = toParse.substr(i + 1, toParse.size());
      break;
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
	struct sockaddr_in6 servaddr;
	socklen_t sockaddr_len = sizeof(servaddr);
	std::vector<int> clifds;
	std::vector<std::string> usernames;
	std::unordered_map<std::string, std::unordered_set<std::string> > channels;
	int numclients = 0;
	int password_set = 0;
	std::string password;
	std::string message;
	int maxfds;
  std::regex pwdAllowed("[a-zA-Z][_0-9a-zA-Z]*");
  std::regex usrAllowed("[a-zA-Z][_0-9a-zA-Z]*");
  std::regex chnlAllowed("#[a-zA-Z][_0-9a-zA-Z]*");
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
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_port = htons(0);
	//servaddr.sin6_addr.s6_addr = htonl(INADDR_ANY);
  servaddr.sin6_addr = in6addr_any;


	//open the socket
	int lsock;
	if ((lsock = socket(PF_INET6,SOCK_STREAM, 0)) < 0) {
		perror("could not create listen socket\n");
		return 1;
	}

	//bind the socket
	if ((bind(lsock, (struct sockaddr *)&servaddr, sockaddr_len)) < 0) {
		perror("failed binding socket\n");
		return 1;
	}

  //allow the ipv6 socket to also take in ipv4
  int no = 0;
  setsockopt(lsock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no));

	//open socket for listening
	if (listen(lsock, 10) < 0) {
		perror("failed to open socket for listening\n");
		return 1;
	}

	//print out new port for the socket
	getsockname(lsock, (struct sockaddr *)&servaddr, &sockaddr_len);
	printf("Port: %d\n",ntohs(servaddr.sin6_port));

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
						message = "There are currently " + std::to_string(channels[command_list[1]].size()) + " members.\n" + channel + "members: ";
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
							if ((regex_match(command_list[1], chnlAllowed)) && command_list[1].size() < 22) {//CHECK REGEX
								std::unordered_set<std::string> emptyset;
								emptyset.insert(usernames[i]);
								channels[command_list[1]] = emptyset;
								message = "Joined channel " + command_list[1] + ".\n";
							} else {
								message = "Invalid channel name.\n";
							}
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
							message = "Joined channel " + command_list[1] + ".\n";
						}
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
								} else {
									message = command_list[2] + " is not in channel " + command_list[1] + ".\n";
								}
							} else {
								message = "Invalid channel name.\n";
							}
						} else { // current user is not an operator
							message = "You must be an operator to kick another user.\n";
						}
					}

					write(clifds[i], message.c_str(), message.length());

				} else if (command == "PRIVMSG") {
					if (command_list.size() < 3) {
						message = "Invalid PRIVMSG command.\n";
					} else if(command_list[2].length() > 512) {
						message = "Message must be no more than 512 bytes.\n";
					} else {
						if (command_list[1][0] == '#') { //message a channel
							if (channels.count(command_list[1]) == 1) {
								int in_channel = 0;
								std::unordered_set<std::string>::iterator itr = channels[command_list[1]].begin();
								for (; itr != channels[command_list[1]].end(); ++itr) {
									if (*itr == usernames[i]) {
										in_channel = 1;
										break;
									}
								}
								if (in_channel) {
									itr = channels[command_list[1]].begin();
									message = command_list[1] + "> " + usernames[i] + ": " + command_list[2] ;
									for (; itr != channels[command_list[1]].end(); ++itr) {
										for (int j=0; j<usernames.size(); j++) {
											if (*itr == usernames[j] && usernames[j] != usernames[i]) {
												write(clifds[j], message.c_str(), message.length());
												break;
											}
										}
									}
								} else {
									message = "You are not in this channel.\n";
								}
							} else {
								message = "Channel does not exist.\n";
							}
						} else { //message a specific user
							int recipient_index;
							int recipient_exists = 0;
							for (int j=0; j<usernames.size(); j++) {
								if (usernames[j] == command_list[1]) {
									recipient_exists = 1;
									recipient_index = j;
									break;
								}
							}

							if (recipient_exists) {
								message = "<<" + usernames[i] + " " + command_list[2];
								write(clifds[recipient_index], message.c_str(), message.length());
								message = command_list[1] + ">> " + command_list[2];
							} else {
								message = "Invalid username for recipient.\n";
							}
						}
					}

					write(clifds[i], message.c_str(), message.length());

				} else if (command == "QUIT") {
					operators.erase(usernames[i]);
					std::unordered_map<std::string, std::unordered_set<std::string> >::iterator itr = channels.begin();
					for (; itr != channels.end(); ++itr) {
						if ((itr->second).erase(usernames[i])) {
							std::unordered_set<std::string>::iterator itr2 = (itr->second).begin();
							message = itr->first + "> " + usernames[i] + " has quit.\n";
							for (;itr2 != (itr->second).end(); ++itr2) {
								for (int j=0; j<usernames.size(); j++) {
									if (usernames[j] == *itr2) {
										write(clifds[j], message.c_str(), message.length());
										break;
									}
								}
							}
						}
					}
					close(clifds[i]);
					clifds.erase(clifds.begin() + i);
					usernames.erase(usernames.begin() + i);
					numclients--;
					i--;

				} else { //INVALID COMMAND
					message = "Invalid command.\n";
					write(clifds[i], message.c_str(), message.length());
				}


			}
		}

	}



}
