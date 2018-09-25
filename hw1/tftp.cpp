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
	int nrec;
	char packet[PACKET_SIZE];
	char m[4];
	struct sockaddr_in ack;
	socklen_t ack_len;

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

		printf("file packet: %s with %d bytes\n", packet, bytesread);
		blocknum++;

		for (; attempts < RETRIES; attempts++) {
			/// send the packet 
			//encode packet

			uint16_t opcode = htons(DATA);
			uint16_t block = htons(blocknum);
			printf("sendop: %d to %d block: %d to %d\n",DATA, opcode, blocknum, block);
			char data[bytesread + 4];
			memcpy(data, (char*)&opcode, 2);
			memcpy(data + 2, (char*)&block, 2);
			memcpy(data + 4, packet, bytesread);

			printf("Packet to send: %s\n", data);

			sendto(clifd, data, 4 + bytesread, 0, (struct sockaddr *)servaddr, sockaddr_length);

			/// receive response
				//if response received, break
			alarm(1);
			nrec = recvfrom(clifd, m, 4, 0, (struct sockaddr *)& servaddr, (socklen_t *)sockaddr_length);
			if (nrec < 0) {
				perror("receive error in RRQ");
				return;
			}
			alarm(0);

			uint16_t* opp = (uint16_t*)m;
			opcode = ntohs(*opp);

			if (opcode == ERROR) {
				printf("Error received from client\n");
				return;
			}

			if (opcode != ACK) {
				printf("invalid message received\n");
				return;
			}
			int bnum = m[2] + m[3];
			printf("Block number: %d\n", bnum);

			if (bnum != blocknum) {
				printf("Invalid block number recieved.\n");
				return;
			}
			else {
				break;
			}

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
	int blocknum = 0, done = 0, attempts = 0, clifd=0;

	if ((clifd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Unable to create client socket\n");
		return;
	}

	//TODO: send ack here
	unsigned char ackpack[4];
	uint16_t opcode = htons(ACK);
	uint16_t block = htons(blocknum);
	memcpy(ackpack, (char*)&opcode, 2);
	memcpy(ackpack + 2, (char*)&block, 2);
	printf("ackpack %s\n", ackpack);
	sendto(clifd, ackpack, 4, 0, (struct sockaddr*)servaddr, sockaddr_length);



	if (!file) {
		perror("File could not be created\n");
		exit(1);
	}

	

	while(!done) {
		std::string messageText = "";
		int opcode;

		int sockfd = 0;;
		int	n;
		socklen_t len;
		char mesg[PACKET_SIZE];
		blocknum++;

		len = sizeof(servaddr);

		for(; attempts < RETRIES; attempts++) {
			// Receive data

			printf("RECEIVED: ");
			n = Recvfrom(clifd, mesg, PACKET_SIZE, 0, (SA *) &servaddr, &len);
			//data will have opcode of 3
			for (int i=0;i<n;i++) {
				//printf("%d",mesg[i]);
				messageText += mesg[i];
				printf("%d", mesg[i]);
				if (i == 1) {
					opcode = mesg[i];
				}
			}

			printf("\n");

				// If received, break
			// Send ack 

			alarm(1);
			if (opcode == DATA) {
				break;
			}
			if (n < PACKET_SIZE) {
				done = 1;
			}
		}

		if(attempts > 9) {
			perror("Retried 10 times, no response ..... aborting\n");
			exit(1);
		}
		unsigned char sendack[4];
		block = htons(blocknum);
		opcode = htons(ACK);
		memcpy(sendack, (char*)&opcode, 2);
		memcpy(sendack + 2, (char*)&block, 2);
		printf("sendack %s\n", sendack);
		
		if (sendto(clifd, ackpack, 4, 0, (struct sockaddr*)servaddr, sockaddr_length) < 0) {
			if(errno == EFAULT)
			perror("error sending ack to client\n");
			exit(1);
		}

		//write data to file here
		fputs(messageText.c_str(), file);


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

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("server socket error\n");
		exit(1);
	}

	sockaddr_length = sizeof(servaddr);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(0);

	//sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	Bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	

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

		n = recvfrom(sockfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len);

		//mesg[n-1] = '\0';
		//std::string message = mesg;
		//printf("Op code num is %s\n", message.c_str());

		//std::cout << "break" << std::endl;

		std::string messageText = "";

		uint16_t opcode;
		uint16_t* opp = (uint16_t*)mesg;
		opcode = ntohs(*opp);



		printf("PACKET: ");
		for (int i=0;i<n;i++)
		{
			printf("%d",mesg[i]);
			messageText += mesg[i];
			// if (i == 1) {
			// 	opcode = mesg[i];
			// }
		}
		
		printf("\n");
		std::string fileName = messageText.substr(2, messageText.find("octet"));
		const char* fname = fileName.c_str();
		//std::cout << "The message text is: " + messageText << std::endl; 
		
		//printf("The file name is %s\n", fileName.c_str());
		//std::cout << "The opcode is:" + int(opcode) << std::endl;
		printf("OPCODE: %d\n", opcode);
		if(opcode != RRQ && opcode != WRQ) {
			perror("wrong\n");
			exit(1);
			// *opp = htons(ERROR);
			// *(opp+1) = htons(4);
			// *(mesg + 4) = 0;

			// sendto(sockfd, mesg, 5, 0, (struct sockaddr *)&servaddr, sockaddr_length);

		}
		else {
			if (fork() == 0) {
				printf("The fileName is: %s\n",fname);
				if (opcode == RRQ) {
					handle_read_request(&cliaddr, len, fname);
				}
				else if (opcode == WRQ) {
					handle_write_request(&cliaddr, len, fname	);
				}

				exit(0);
			}
		}

		//std::cout << "n: " + n << std::endl;
		//std::cout << "sockfd: " + sockfd << std::endl;
		//std::cout << "n: "<< n << std::endl;
		//std::cout << "msg: " + std::string(mesg) << std::endl;
		//printf("%s", mesg);


	}



	close(sockfd);
	return 0;
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
