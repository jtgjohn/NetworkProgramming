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
#include <openssl/evp.h>
#include <vector>
#include <list>
#include <sys/time.h>
#include <math.h>
#include <utility>
#include <algorithm>


#define MAXLINE 1024

typedef struct {
	std::string name;
	int port;
	uint8_t id;
	int idset;
} Node;

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

void print_kbuckets(const std::vector<std::list< Node > > &kb) {
	for (int i=0; i<kb.size(); i++) {
		printf("Bucket %i:", i);
		std::list<Node>::const_iterator itr;
		for (itr = kb[i].begin(); itr != kb[i].end(); ++itr) {
			printf(" %x", itr->id);
		}
		printf("\n");
	}
}

int node_in_buckets(const std::vector<std::list<Node > > &kb, uint8_t myid, const Node & n) {
	int bucknum = log2(n.id^myid);
	if (bucknum < 0) {
		bucknum = 0;
	}
	int in = 0;
	std::list<Node>::const_iterator itr;
	for (itr = kb[bucknum].begin(); itr != kb[bucknum].end(); ++itr) {
		if (itr->id == n.id) {
			in = 1;
			break;
		}
	}
	return in;
}

std::vector<std::pair<int, uint8_t> > find_k_closest(const std::vector<std::list<Node> > &kb, uint8_t id, int k) {
	std::vector<std::pair<int, uint8_t> > closest; 
	//create vector of pairs of xor and bucknum, then sort
	//add first k bucknum to closest where xor matches
	std::vector<std::pair<int, int> > xor_sort;
	for (int i=0; i<kb.size(); i++) {
		std::list<Node>::const_iterator itr;
		for (itr = kb[i].begin(); itr != kb[i].end(); ++itr) {
			std::pair<int, int> temp_pair = std::make_pair(id^itr->id, i);
			xor_sort.push_back(temp_pair);
		}
	}

	//sort the vector based on xor distance
	std::sort(xor_sort.begin(), xor_sort.end());

	//only go up to k values or if there are less than k values
	int uptok = std::max((int)xor_sort.size(), k);

	//add up to k values to the vector of closest matches and return it
	for (int i=0; i<uptok; i++) {
		std::list<Node>::const_iterator itr;
		for (itr=kb[xor_sort[i].second].begin(); itr != kb[xor_sort[i].second].end(); ++itr) {
			if (((itr->id)^id) == (xor_sort[i].first)) {
				std::pair<int, uint8_t> temp_pair = std::make_pair(xor_sort[i].second, itr->id);
				closest.push_back(temp_pair);
				break;
			}
		}
	}

	return closest;
}

int main(int argc, char* argv[]) {
	if (argc != 5) {
		std::cout << "Usage: ./dht.out <nodeName> <port> <nodeIDseet> <k>\n";
		exit(1);
	}
	std::string nodeName = argv[1];
	std::string portstr = argv[2];
	int port = atoi(argv[2]);
	int k = atoi(argv[4]);
	socklen_t sockaddr_len;
	int sockfd;
	struct sockaddr_in addr;
	int finished_command = 1;
	std::vector<std::list<Node > > kbuckets;
	for (int i=0; i<8; i++) {
		std::list<Node> t;
		kbuckets.push_back(t);
	}

	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char id[EVP_MAX_MD_SIZE];
	unsigned int md_len;

	//Create a context
	mdctx = EVP_MD_CTX_create();

	//Inititialize our digest to use the md TYPE object given by EVP_sha1
	//Need to do this everytime we want to hash something
	EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);

	//Add to the hash
	EVP_DigestUpdate(mdctx, argv[3], strlen(argv[3]));

	//Finalize the hash, store in md_value
	EVP_DigestFinal_ex(mdctx, id, &md_len);

	//Free the context
	EVP_MD_CTX_destroy(mdctx);

	uint8_t myid = id[0];

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("server socket error\n");
		exit(1);
	}
	sockaddr_len = sizeof(addr);
	memset(&addr, 0 , sockaddr_len) ;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
  inet_pton(AF_INET, argv[1], &addr.sin_addr);

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

		/*
		struct timeval {
			long tv_sec; //seconds
			long tv_usec; //microseconds
		};
		*/
		struct timeval timeout;
		timeout.tv_sec = 1;

		select(maxfds, &rset, NULL, NULL, &timeout);

		if (finished_command && (FD_ISSET(fileno(stdin), &rset))) {

			char buffer[MAXLINE];
			int n = read(fileno(stdin), buffer, MAXLINE);

			std::string input;
			input.assign(buffer, n);

			printf("Command: %s", buffer);
			std::vector<std::string> command_list = parser(input);
			if (command_list[0] == "CONNECT") {
				struct sockaddr_in send;
				memset(&send, 0, sizeof(send));
				send.sin_family = AF_INET;
				send.sin_port = htons(atoi(command_list[2].c_str()));
				inet_pton(AF_INET, command_list[1].c_str(), &(send.sin_addr));
				std::string message = "HELLO " + nodeName + " " + std::to_string(myid) + "\n";
				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&send, sizeof(send));
			}
			if (command_list[0] == "FIND_NODE") {
				std::vector<std::pair<int, uint8_t> > closek = find_k_closest(kb,command_list[1], k);
				for (int i=0; i<closek.size(); i++) {
					
				}
			}
		}
		if (FD_ISSET(sockfd, &rset)) {

			char buffer[MAXLINE];
			struct sockaddr_in recvaddr;
			socklen_t len = sizeof(recvaddr);
			std::string message;


			int n = recvfrom(sockfd, buffer, MAXLINE, 0 , (struct sockaddr *)&recvaddr, &len);

			std::string input;
			input.assign(buffer, n);


			std::vector<std::string> message_list = parser(input);
			printf("Message: %s", input.c_str());
			if (message_list[0] == "HELLO") {
				message = "MYID " + std::to_string(myid) + "\n";
				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
				Node newNode;
				newNode.id = atoi(message_list[2].c_str());
				newNode.port = ntohs(recvaddr.sin_port);
				newNode.name = message_list[1];
				int bucknum = log2(newNode.id^myid);

				//This should only happen when they are the same node
				if (bucknum < 0) {
					bucknum = 0;
				}
				if (!node_in_buckets(kbuckets, myid, newNode)) {
					if (kbuckets[bucknum].size() >= k) {
						kbuckets[bucknum].pop_front();
					} 
					kbuckets[bucknum].push_back(newNode);
					print_kbuckets(kbuckets);
				}
			}

			if (message_list[0] == "MYID") {
				Node newNode;
				newNode.id = atoi(message_list[1].c_str());
				newNode.port = ntohs(recvaddr.sin_port);
				char buf[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(recvaddr.sin_addr), buf, INET_ADDRSTRLEN);
				newNode.name.assign(buf, INET_ADDRSTRLEN);
				int bucknum = log2(newNode.id^myid);
				//This should only happen when they are the same node
				if (bucknum < 0) {
					bucknum = 0;
				}
				if (!node_in_buckets(kbuckets, myid, newNode)) {
					if (kbuckets[bucknum].size() >= k) {
						kbuckets[bucknum].pop_front();
					} 
					kbuckets[bucknum].push_back(newNode);
					print_kbuckets(kbuckets);
				}
			}

			if (message_list[0] == "FIND_NODE") {

			}

			if (message_list[0] == "NODE") {

			}

		}

	}
}