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


void handle_read_request(sockaddr_in* servaddr, socklen_t sockaddr_length, const char* fname) {
	in_port_t cli_port = servaddr->sin_port;
	FILE *file;
	int clifd=0,blocknum = 0, timeout = 0, attempts = 0, done = 0, bytesread = 0;
	char* packet[PACKET_SIZE];

	if((clifd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
		printf("Client socket unable to be created\n");
		return;
	}

	file = fopen(fname, "r");
	if (!file) {
		perror("Invalid file\n");
		exit(1);
	}

	while (!done) {
		bytesread = fread(packet, 1, PACKET_SIZE, file);

		if (bytesread < PACKET_SIZE) { //last packet of data to send
			done = 1;
		}

		blocknum++;

		for (; attempts < RETRIES; attempts++) {
			/// send the packet 
			//encode packet
			uint16_t opcode = htons(DATA);
			uint16_t block = htons(blocknum);
			char data[PACKET_SIZE + 4 + 1];
			memcpy(data, (char*)&opcode, 2);
			memcpy(data + 2, (char*)&block, 2);
			memcpy(data + 4, packet, bytesread);

			sendto(clifd, packet, 4 + bytesread, 0, (struct sockaddr *)servaddr, sockaddr_length);

			/// receive response
				//if response received, break
			//receive_packet()
			alarm(1);
		}

		if (attempts > 9) {
			perror("Retried 10 times, no response ..... aborting\n");
			exit(1);
		}

	}


	fclose(file);
}

void handle_write_request(sockaddr_in* servaddr, socklen_t sockaddr_length, const char* fname) {
	FILE* file = fopen(fname, "w");
	int blocknum = 0, done = 0, attempts = 0;

	//TODO: send ack here

	if (!file) {
		perror("File could not be created\n");
		exit(1);
	}


	while(!done) {
		for(; attempts < RETRIES; attempts++) {
			// Receive data
				// If received, break
			// Send ack 
			alarm(1);
		}

		if(attempts > 9) {
			perror("Retried 10 times, no response ..... aborting\n");
			exit(1);
		}

		//TDOD: write data here


	}

	fclose(file);

}



void child_signal(int s) {
	pid_t pid;
	int stat;
	while((pid = waitpid(-1,&stat,WNOHANG)) > 0);
}

void alarm_signal(int s) {
	printf("SIGNAL ALARM\n");
	alarm(1);
}


int main(int argc, char **argv)
{
	socklen_t sockaddr_length;

	int sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	Signal(SIGCHLD, child_signal);
	Signal(SIGALRM, alarm_signal);

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

	while(1) {
		len = sizeof(cliaddr);
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len);

		mesg[n-1] = '\0';
		//std::string message = mesg;
		//printf("Op code num is %s\n", message.c_str());

		//std::cout << "break" << std::endl;

		std::string messageText = "";

		int opcode;

		for (int i=0;i<n;i++)
		{
			//printf("%d",mesg[i]);
			messageText += mesg[i];
			if (i == 1) {
				opcode = mesg[i];
			}
		}
		
		//printf("\n");
		std::string fileName = messageText.substr(0, messageText.find("octet"));

		//std::cout << "The message text is: " + messageText << std::endl; 
		printf("Temp op code: %d\n", opcode);
		std::cout << "The fileName is: " + fileName << std::endl;
		//printf("The file name is %s\n", fileName.c_str());
		//std::cout << "The opcode is:" + int(opcode) << std::endl;
		if(opcode != RRQ && opcode != WRQ) {
			perror("INVALID PACKET TYPE");
			exit(1);
		}
		else {
			if (fork() == 0) {
				if (opcode == RRQ) {
					handle_read_request(&servaddr, sockaddr_length, fileName.c_str());
				}
				else if (opcode == WRQ) {
					handle_write_request(&servaddr, sockaddr_length, fileName.c_str()	);
				}

				close(sockfd);
				return 0;
			}
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
