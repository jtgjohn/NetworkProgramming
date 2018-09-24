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

#define TIMEOUT 1
#define RETRIES 10

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

#define PACKET_SIZE 512


extern "C" {
	#include	"../unpv13e/lib/unp.h"
}


void handle_read_request(sockaddr_in* servaddr, socklen_t sockaddr_length, char* fname) {
	in_port_t cli_port = servaddr->sin_port;
	FILE *file;
	int blocknum = 0, timeout = 0, attempts = 0, closed = 0, bytesread = 0;
	//int fd = open(fname, O_RDONLY);
	char packet[PACKET_SIZE];


	file = fopen(fname, "r");
	if (!file) {
		perror("Invalid file");
		exit(1);
	}

	while (!closed) {
		bytesread = fread(packet, 1, PACKET_SIZE, file);

		if (bytesread < PACKET_SIZE) { //last packet of data to send
			closed = 1;
		}

		for (; attempts < RETRIES; attempts++) {
			/// resend the packet 
			/// receive response
		}

		if (attempts > 9) {
			perror("Retried 10 times, no response ..... aborting\n");
			exit(1);
		}

	}



}

void handle_write_request(sockaddr_in* servaddr, socklen_t sockaddr_length, char* fname) {

}


void ack() {

}

void recieve_packet() {

}

void send_packet() {

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
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(0);//htons(9877);//htons(0);

	//sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	Bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));//sockaddr_length);

	getsockname(sockfd, (struct sockaddr *)&servaddr, &sockaddr_length);

	int portNum = ntohs(servaddr.sin_port);

	printf("Port: %d\n", portNum);




	///TODO: Read client request, store opcode and filename into  variables

	//server is up, start waiting for a message
	//dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];

	for ( ; ; ) {
		len = sizeof(cliaddr);
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len);

		mesg[n-1] = '\0';
		//std::string message = mesg;
		//printf("Op code num is %s\n", message.c_str());

		//std::cout << "break" << std::endl;

		std::string messageText = "";

		int opCode;

		for (int i=0;i<n;i++)
		{
			//printf("%d",mesg[i]);
			messageText += mesg[i];
			if (i == 1) {
				opCode = mesg[i];
			}
		}
		
		//printf("\n");
		std::string fileName = messageText.substr(0, messageText.find("netascii"));

		//std::cout << "The message text is: " + messageText << std::endl; 
		printf("Temp op code: %d\n", opCode);
		std::cout << "The fileName is: " + fileName << std::endl;
		//printf("The file name is %s\n", fileName.c_str());
		//std::cout << "The opcode is:" + int(opCode) << std::endl;

		if (opCode == 1) {
			std::cout << "READ" << std::endl;
		}else if (opCode == 2) {
			std::cout << "WRITE" << std::endl;
		}else{
			std::cout << "ERROR" << std::endl;
		}

		//std::cout << "n: " + n << std::endl;
		//std::cout << "sockfd: " + sockfd << std::endl;
		//std::cout << "n: "<< n << std::endl;
		//std::cout << "msg: " + std::string(mesg) << std::endl;
		//printf("%s", mesg);


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
