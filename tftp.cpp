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

#define TIMEOUT 1
#define RETRIES 10

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

#define PACKET_SIZE 512

#define MODE "octet"




extern "C" {
	#include	"unpv13e/lib/unp.h"
}

void read_request() {

}

void write_request() {

}



void child_signal(int s) {
	pid_t pid;
	int stat;
	while((pid = waitpid(-1,&stat,WNOHANG)) > 0);
}




int main(int argc, char **argv)
{
	socklen_t sockaddr_length;
	int sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_length = sizeof(servaddr);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = PF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(0);

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	bind(sockfd, (struct sockaddr *)&servaddr, sockaddr_length);

	getsockname(sockfd, (struct sockaddr *)&servaddr, &sockaddr_length);


	printf("Port: %d\n", servaddr.sin_port);



	dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

}