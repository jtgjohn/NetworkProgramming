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




int main(int argc, char* argv[]) {
	struct sockaddr_in servaddr;
	socklen_t sockaddr_len = sizeof(servaddr);
	std::vector<int> clifds(FD_SETSIZE, -1);
	std::vector<std::string> usernames(FD_SETSIZE, "");
	std::unordered_map<std::string, std::vector<std::string> > channels;
	std::unordered_map<std::string, std::vector<std::string> > user_channels;






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



}