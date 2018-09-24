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

#define TIMEOUT 1
#define RETRIES 10

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

#define PACKET_SIZE 512

#define MODE "octet"

typedef struct {
	char file[PACKET_SIZE];
} read_request;


extern "C" {
	#include	"unpv13e/lib/unp.h"
}




void handle_read_request(sockaddr_in* servaddr, socklen_t sockaddr_length, char* fname) {
	in_port_t cli_port = servaddr->sin_port;
	FILE *file;


	file = fopen(fname, "r");

	//google open(file, XXXXXXXreadonly)
	//lseek()


}

void write_request(sockddr_in* servaddr, socklen_t sockaddr_length, char* fname) {

}


void ack()

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
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(0);//htons(9877);//htons(0);

	//sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	Bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));//sockaddr_length);

	getsockname(sockfd, (struct sockaddr *)&servaddr, &sockaddr_length);

	int portNum = ntohs(servaddr.sin_port);

	printf("Port: %d\n", portNum);


	//server is up, start waiting for a message
	//dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];

	for ( ; ; ) {
		len = sizeof(cliaddr);
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len);
		
		std::cout << "n: " + n << std::endl;
		std::cout << "sockfd: " + sockfd << std::endl;
		std::cout << "n: "<< n << std::endl;
		std::cout << "msg: " + std::string(mesg) << std::endl;
		printf("%s", mesg);

		//Sendto(sockfd, mesg, n, 0, pcliaddr, clilen);
	}
	

}

/*
dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];

	for ( ; ; ) {
		len = clilen;
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

		Sendto(sockfd, mesg, n, 0, pcliaddr, clilen);
	}
}

*/
