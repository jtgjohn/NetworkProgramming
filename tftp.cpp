#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <string>

extern "C" {
	#include	"unpv13e/lib/unp.h"
}

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;
	socklen_t addrlen = sizeof(servaddr);

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = 0;//htons(SERV_PORT);


	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	//printf("%d", servaddr.sin_port);
	//std::cout << getsockname() << std::endl;

	getsockname(sockfd, (struct sockaddr *)&servaddr, &addrlen); // read binding

	int local_port = ntohs(servaddr.sin_port);  // get the port number
	int temp = 1;
	std::cout << "Port Number is " + temp << std::endl;
	/*
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(servaddr, (struct sockaddr *)&sin, &len) == -1) {
    	//perror("getsockname");
    	std::cout << "Machine broke" << std::endl;
	}
	else {
    	//printf("port number %d\n", ntohs(sin.sin_port));
    	std::cout << "Port Number is " + ntohs(sin.sin_port) << std::endl;
	}
	*/
	
	dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	
}