#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h>

extern "C" {
	#include	"unp.h"
}

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	printf("%d", servaddr.sin_port);
	printf("Ahoy there");

	//dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
}