#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <vector>
#include <list>
#include <tuple>



int main(int argc, char* argv[]) {
	if (argc != 5) {
		std::cout << "Usage: ./dht.out <nodeName> <port> <nodeIDseet> <k>\n";
		exit(1);
	}
	std::string nodeName = argv[1];
	int port = argv[2];
	int seed = argv[3];
	int k = argv[4];
	socklen_t sockaddr_len;
	int sockfd
	struct sockaddr_in addr;
	int finished_command = 1;
	std::vector<std::list<std::tuple<std::string, int, int> > > kbuckets;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("server socket error\n");
		exit(1);
	}
	sockaddr_len = sizeof(addr);
	memset(&addr, 0 , sockaddr_len);
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_port = port;
  servaddr.sin6_addr = in6addr_any;

  if ((bind(sockfd, (struct sockaddr*)&addr, sockaddr_len)) < 0) {
  	perror("failed binding socket\n");
  	return 1;
  }

  maxfds = sockfd;

  fd_set rset;
	while(1) {
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		FD_SET(fileno(stdin),&rset);
		maxfds = std::max(maxfds, fileno(stdin));
		maxfds++;


		select(maxfds, &rset, NULL, NULL, NULL);

		if (finished_command && (FD_ISSET(fileno(stdin), &rset))) {

			if (command_list[0] == "CONNECT") {

			}
		}
		if (FD_ISSET(sockfd, &rset)) {

			if (message_list[0] == "HELLO") {

			}

			if (message_list[0] == "MYID") {

			}

		}

	}
}