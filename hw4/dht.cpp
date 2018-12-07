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
#include <tuple>


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
	int port = atoi(argv[2]);
	std::string seed = argv[3];
	int k = atoi(argv[4]);
	socklen_t sockaddr_len;
	int sockfd;
	struct sockaddr_in addr;
	int finished_command = 1;
	std::vector<std::list<std::tuple<std::string, int, int> > > kbuckets;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("server socket error\n");
		exit(1);
	}
	sockaddr_len = sizeof(addr);
	memset(&addr, 0 , sockaddr_len);
	addr.sin_family = AF_INET;
	addr.sin_port = port;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

			command_list = parser();
			if (command_list[0] == "CONNECT") {

			}
		}
		if (FD_ISSET(sockfd, &rset)) {
			
			message_list = parser();
			if (message_list[0] == "HELLO") {

			}

			if (message_list[0] == "MYID") {

			}

		}

	}
}