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
#include <openssl/sha.h>
#include <vector>
#include <list>
#include <sys/time.h>


#define MAXLINE 1024

typedef struct {
	std::string name;
	int port;
	uint8_t id;
	int idset;
} Node;

//function to parse input sent to the server
std::vector<std::string> parser(std::string toParse) {

  //vector to hold the parse message
  std::vector<std::string> parsed;

  //temp variable to hold parts of the message that will be added to toParse
  std::string toAdd = "";

  //go through the message one char at a time
  for (int i = 0; i < toParse.size(); i++) {
    //if the character isn't a space, add the char to toAdd
    if (!(isspace(toParse.at(i)))) {
      toAdd += toParse.at(i);
    //else the char is a space, add the word to parsed and reset toAdd
    }else{
      parsed.push_back(toAdd);
      toAdd = "";
    }
  }
  //add the last word
  parsed.push_back(toAdd);

  //get rid of any extra white space (in case the user enters multiple spaces between words)
  for (int i = 0; i < parsed.size(); i++) {
    if (parsed[i] == "") {
      parsed.erase(parsed.begin() + i);
      i--;
    }
  }

  //return the parsed vecotr
  return parsed;

}

int main(int argc, char* argv[]) {
	if (argc != 5) {
		std::cout << "Usage: ./dht.out <nodeName> <port> <nodeIDseet> <k>\n";
		exit(1);
	}
	std::string nodeName = argv[1];
	std::string portstr = argv[2];
	int port = atoi(argv[2]);
	std::string seed = argv[3];
	int k = atoi(argv[4]);
	socklen_t sockaddr_len;
	int sockfd;
	struct sockaddr_in addr;
	int finished_command = 1;
	std::vector<std::list<Node > > kbuckets;


	uint8_t myid;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("server socket error\n");
		exit(1);
	}
	sockaddr_len = sizeof(addr);
	memset(&addr, 0 , sockaddr_len);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
  inet_pton(AF_INET, argv[1], &addr.sin_addr);

  if ((bind(sockfd, (struct sockaddr*)&addr, sockaddr_len)) < 0) {
  	perror("failed binding socket\n");
  	return 1;
  }

  int maxfds = sockfd;

  fd_set rset;
	while(1) {
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		FD_SET(fileno(stdin),&rset);
		maxfds = std::max(maxfds, fileno(stdin));
		maxfds++;


		select(maxfds, &rset, NULL, NULL, NULL);

		if (finished_command && (FD_ISSET(fileno(stdin), &rset))) {

			char buffer[MAXLINE];
			int n = read(fileno(stdin), buffer, MAXLINE);

			std::string input;
			input.assign(buffer, n);

			std::vector<std::string> command_list = parser(input);
			if (command_list[0] == "CONNECT") {
				std::string message = "HELLO " + nodeName + " " + portstr + "\n";
				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&addr, sockaddr_len);
			}
		}
		if (FD_ISSET(sockfd, &rset)) {

			char buffer[MAXLINE];
			struct sockaddr_in recvaddr;
			socklen_t len = sizeof(recvaddr);
			std::string message;


			int n = recvfrom(sockfd, buffer, MAXLINE, 0 , (struct sockaddr *)&recvaddr, &len);

			std::string input;
			input.assign(buffer, n);


			std::vector<std::string> message_list = parser(buffer);
			if (message_list[0] == "HELLO") {
				message = "MYID " + nodeName + "\n";
				Node newNode;
				newNode.id = atoi(message_list[2].c_str());
				newNode.port = ntohs(recvaddr.sin_port);
				newNode.name = message_list[1];
			}

			if (message_list[0] == "MYID") {
				Node newNode;
				newNode.id = atoi(message_list[1].c_str());
				newNode.port = ntohs(recvaddr.sin_port);
				char buf[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(recvaddr.sin_addr), buf, INET_ADDRSTRLEN);
				newNode.name.assign(buf, INET_ADDRSTRLEN);

			}

		}

	}
}