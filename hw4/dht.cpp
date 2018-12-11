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

//This function is to help visualize what is happening inside the 
//distributed hash table. It prints out each bucket and its contents.
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

//This function returns a boolean. 
//If the provided node is in the kbuckets, then it returns true else returns false
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


//This function implemments the search functionality of the distributed hash table
//When looking for a given id, we are looking for the closest distance in the kbuckets
//Up to k elements are returned, and this function returns those with the least
//distance from the given id. 
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
	int uptok = std::min((int)xor_sort.size(), k);

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
	std::pair<int, int> store_value = std::make_pair(0,0);
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
  std::vector<std::pair<int, uint8_t> > send_find_node;
  std::vector<std::pair<int, uint8_t> > send_find_value;
  uint8_t lookingfornode;
  int lookingforkey;
  int sendmore = 0;
  int sentcount = 0;
  int waitfornode = 0;
  int waitforvalue = 0;

  fd_set rset;


  //Main loop of the function, based on a select call
	while(1) {
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		FD_SET(fileno(stdin),&rset);
		maxfds = std::max(maxfds, fileno(stdin));
		maxfds++;

		//if we need to record  timeout, make the struct
		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		int tout;

		//if we are waiting for a message, set the timeout select
		if (waitfornode || waitforvalue) {
			tout = select(maxfds, &rset, NULL, NULL, &timeout);
		}
		else { //otherwise we can run select with no timeout
			tout = select(maxfds, &rset, NULL, NULL, NULL);
		}

		//if we timed out, then we will send the next node a message
		//depending on what we are looking for
		//If there are no nodes left to send to, we move on
		if (tout == 0) {
			int bucknum = send_find_node[sentcount].first;
			uint8_t findid = send_find_node[sentcount].second;
			std::list<Node>::iterator itr;
			for (itr=kbuckets[bucknum].begin(); itr != kbuckets[bucknum].end(); ++itr) {
				if (itr->id == findid) {
					break;
				}
			}
			struct sockaddr_in send;
			memset(&send, 0, sizeof(send));
			send.sin_family = AF_INET;
			send.sin_port = htons(itr->port);
			inet_pton(AF_INET, (itr->name).c_str(), &(send.sin_addr));

			std::string message = "FIND_NODE " + std::to_string(lookingfornode) +"\n";

			sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&send, sizeof(send));
			sentcount++;
			if (sentcount == send_find_node.size()) {
				sendmore = 0;
				waitfornode = 0;
			}
			continue;
		}

		//if we receive a command, handle it
		if (finished_command && (FD_ISSET(fileno(stdin), &rset))) {

			char buffer[MAXLINE];
			memset(buffer, 0 , MAXLINE);
			int n = read(fileno(stdin), buffer, MAXLINE);

			std::string input;
			input.assign(buffer, n);

			std::vector<std::string> command_list = parser(input);

			//if we receive a connect command,
			//send a hello message to the provided port and address
			if (command_list[0] == "CONNECT") {
				struct sockaddr_in send;
				memset(&send, 0, sizeof(send));
				send.sin_family = AF_INET;
				send.sin_port = htons(atoi(command_list[2].c_str()));
				inet_pton(AF_INET, command_list[1].c_str(), &(send.sin_addr));
				std::string message = "HELLO " + nodeName + " " + std::to_string(myid) + "\n";
				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&send, sizeof(send));
			}

			//if we recevie a find node command, send out k find node messages 
			//to the k closest nodes
			if (command_list[0] == "FIND_NODE") {
				send_find_node = find_k_closest(kbuckets,atoi(command_list[1].c_str()), k);
				int bucknum = send_find_node[0].first;
				uint8_t findid = send_find_node[0].second;
				std::list<Node>::iterator itr;
				for (itr=kbuckets[bucknum].begin(); itr != kbuckets[bucknum].end(); ++itr) {
					if (itr->id == findid) {
						break;
					}
				}
				struct sockaddr_in send;
				memset(&send, 0, sizeof(send));
				send.sin_family = AF_INET;
				send.sin_port = htons(itr->port);
				inet_pton(AF_INET, (itr->name).c_str(), &(send.sin_addr));

				lookingfornode = atoi(command_list[1].c_str());
				std::string message = "FIND_NODE " + command_list[1] +"\n";

				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&send, sizeof(send));
				
				sentcount = 1;
				waitfornode = 1;
				if (sentcount == send_find_node.size()) {
					waitfornode = 0;
					sendmore = 0;
				}
				
			}

			//similar to find node, a find data command will send out 
			//a find data message to the k closest nodes
			if (command_list[0] == "FIND_DATA") {
				int key = atoi(command_list[1].c_str());
				send_find_value = find_k_closest(kbuckets, key, k);
				int bucknum = send_find_value[0].first;
				uint8_t findid = send_find_value[0].second;
				std::list<Node>::iterator itr;
				for (itr=kbuckets[bucknum].begin(); itr != kbuckets[bucknum].end(); ++itr) {
					if (itr->id == findid) {
						break;
					}
				}
				struct sockaddr_in send;
				memset(&send, 0, sizeof(send));
				send.sin_family = AF_INET;
				send.sin_port = htons(itr->port);
				inet_pton(AF_INET, (itr->name).c_str(), &(send.sin_addr));

				std::string message = "FIND_DATA " + command_list[1] + "\n";
				lookingforkey = atoi(command_list[1].c_str());

				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&send, sizeof(send));

				sentcount = 1;
				waitforvalue = 1;
				if (sentcount == send_find_value.size()) {
					waitforvalue = 0;
					sendmore = 0;
				}

			}

			//if we receive a store command, we send out a store command 
			//to the node that matches the key most closely
			if (command_list[0] == "STORE") {
				int key = atoi(command_list[1].c_str());
				int value = atoi(command_list[2].c_str());
				std::vector<std::pair<int, uint8_t> > tostorelist = find_k_closest(kbuckets, key, k);
				if (tostorelist.size() != 0) {
					std::list<Node>::iterator itr = kbuckets[tostorelist[0].first].begin();
					for (; itr != kbuckets[tostorelist[0].first].end(); ++itr) {
						if (itr->id == tostorelist[0].second) {
							std::string message = "STORE " + command_list[1] + " " + command_list[2] + "\n";
							struct sockaddr_in sendstore;
							memset(&sendstore, 0, sizeof(sendstore));
							sendstore.sin_family = AF_INET;
							sendstore.sin_port = htons(itr->port);
							inet_pton(AF_INET, (itr->name).c_str(), &(sendstore.sin_addr));

							sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&sendstore, sizeof(sendstore));
						}
					}
				}
			} 


			//if we receive a quit command, send out to all of our connections 
			//that we are quitting, and then quit
			if (command_list[0] == "QUIT") {
				std::string message = "QUIT " + std::to_string(myid) + "\n";
				for (int i=0; i<kbuckets.size(); i++) {
					std::list<Node>::iterator itr;
					for (itr = kbuckets[i].begin(); itr != kbuckets[i].end(); ++itr) {
						if (itr->id != myid) {
							struct sockaddr_in sendquit;
							memset(&sendquit, 0, sizeof(sendquit));
							sendquit.sin_family = AF_INET;
							sendquit.sin_port = htons(itr->port);
							inet_pton(AF_INET, (itr->name).c_str(), &(sendquit.sin_addr));

							sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&sendquit, sizeof(sendquit));


						}
					}
				}

				close(sockfd);
				return 0;
			}

		}

		//if we receive a message from another node, handle it
		if (FD_ISSET(sockfd, &rset)) {

			char buffer[MAXLINE];
			memset(buffer, 0, MAXLINE);
			struct sockaddr_in recvaddr;
			socklen_t len = sizeof(recvaddr);
			std::string message;


			int n = recvfrom(sockfd, buffer, MAXLINE, 0 , (struct sockaddr *)&recvaddr, &len);

			std::string input;
			input.assign(buffer, n);


			std::vector<std::string> message_list = parser(input);

			//if we receive a hello command, add it to the kbuckets and then send a myid message 
			if (message_list[0] == "HELLO") {
				message = "MYID " + std::to_string(myid) + "\n";
				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
				Node newNode;
				newNode.id = atoi(message_list[2].c_str());
				newNode.port = ntohs(recvaddr.sin_port);
				newNode.name = message_list[1];
				int bucknum = log2(newNode.id^myid);

				printf("Message: HELLO %s %x\n", message_list[1].c_str(), newNode.id);

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

			//if we receive a myid, add it to our kbuckets
			if (message_list[0] == "MYID") {
				Node newNode;
				newNode.id = atoi(message_list[1].c_str());
				newNode.port = ntohs(recvaddr.sin_port);
				char buf[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(recvaddr.sin_addr), buf, INET_ADDRSTRLEN);
				newNode.name.assign(buf, INET_ADDRSTRLEN);
				int bucknum = log2(newNode.id^myid);

				printf("Message: MYID %x\n", newNode.id);
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

			//if we receive a find node message
			//send back the k closest nodes to the node in the message
			if (message_list[0] == "FIND_NODE") {
				uint8_t yourid = atoi(message_list[1].c_str());
				printf("Message: FIND_NODE %x\n", yourid);
				std::vector<std::pair< int, uint8_t> > retnodes = find_k_closest(kbuckets, yourid, k);
				std::string m =  "";
				for (int i=0; i<retnodes.size(); i++) {
					std::list<Node>::iterator itr = kbuckets[retnodes[i].first].begin();
					for (; itr != kbuckets[retnodes[i].first].end(); ++itr) {
						if (itr->id == retnodes[i].second) {
							std::string nodeinfo = "NODE " + itr->name + " " + std::to_string(itr->port) + " " + std::to_string(itr->id) + "\n";
							m += nodeinfo;
							break;
 						}
					}
				}
				sendto(sockfd, m.c_str(), m.length(), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
			}

			//if we receive a node command, we add it to our kbuckets
			if (message_list[0] == "NODE") {
				int n = 0;
				sendmore = 1;
				while (n < message_list.size()) {
					Node newNode;
					newNode.name = message_list[n+1];
					newNode.port = atoi(message_list[n+2].c_str());
					newNode.id = atoi(message_list[n+3].c_str());

					printf("Message: NODE %s %d %x\n", newNode.name.c_str(), newNode.port, newNode.id);
					if (waitfornode && (newNode.id == lookingfornode)) {
						sendmore = 0;
						waitfornode = 0;
					}
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
					n += 4;
				}
			}

			//if we recieve a find data message
			//if we have the data  that they are looking for, return the value
			//otherwise we send back the k closest nodes 
			if (message_list[0] == "FIND_DATA") {
				int key = atoi(message_list[1].c_str());

				printf("Message: FIND_DATA %x\n", key);
				if (key == store_value.first) {
					std::string message = "VALUE " + std::to_string(myid) + " " + std::to_string(key) + " " + std::to_string(store_value.second) + "\n";
					sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
				} else {
					std::vector<std::pair< int, uint8_t> > retnodes = find_k_closest(kbuckets, key, k);
					std::string m =  "";
					for (int i=0; i<retnodes.size(); i++) {
						std::list<Node>::iterator itr = kbuckets[retnodes[i].first].begin();
						for (; itr != kbuckets[retnodes[i].first].end(); ++itr) {
							if (itr->id == retnodes[i].second) {
								std::string nodeinfo = "NODE " + itr->name + " " + std::to_string(itr->port) + " " + std::to_string(itr->id) + "\n";
								m += nodeinfo;
								
								break;
	 						}
						}
					}
					sendto(sockfd, m.c_str(), m.length(), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
				}
			}

			//if we receive a value, we have found what we are looking for and print it out
			if (message_list[0] == "VALUE") {
				printf("Received %d from %x\n",atoi(message_list[3].c_str()), atoi(message_list[1].c_str()));
			}

			//if we receive a store message, store the node as our value
			if (message_list[0] == "STORE") {
				int key = atoi(message_list[1].c_str());
				int value = atoi(message_list[2].c_str());
				printf("Message: STORE %x %d\n", key, value);
				store_value = std::make_pair(key, value);
			}

			//if we recevie a quit message, remove that node from our kbuckets.
			if (message_list[0] == "QUIT") {
				uint8_t yourid = atoi(message_list[1].c_str());
				printf("Message: QUIT %x\n", yourid);
				int bucknum = log2(myid^yourid);
				//This should only happen when they are the same node
				if (bucknum < 0) {
					bucknum = 0;
				}

				std::list<Node>::iterator itr;
				for (itr = kbuckets[bucknum].begin(); itr != kbuckets[bucknum].end(); ++itr) {
					if (itr->id == yourid) {
						kbuckets[bucknum].erase(itr);
						break;
					}
				}
			}

			//if we need to keep sending nodes a find node request, send it out to the 
			//next node in the list
			if (sendmore && waitfornode) {
				int bucknum = send_find_node[sentcount].first;
				uint8_t findid = send_find_node[sentcount].second;
				std::list<Node>::iterator itr;
				for (itr=kbuckets[bucknum].begin(); itr != kbuckets[bucknum].end(); ++itr) {
					if (itr->id == findid) {
						break;
					}
				}
				struct sockaddr_in send;
				memset(&send, 0, sizeof(send));
				send.sin_family = AF_INET;
				send.sin_port = htons(itr->port);
				inet_pton(AF_INET, (itr->name).c_str(), &(send.sin_addr));

				std::string message = "FIND_NODE " + std::to_string(lookingfornode) +"\n";

				sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr *)&send, sizeof(send));
				sentcount++;
				if (sentcount == send_find_node.size()) {
					waitfornode = 0;
					sendmore = 0;
				}
			}

			//if we need to keep sending out request for find data, send it out to the next
			if (sendmore && waitforvalue) {
				int bucknum = send_find_value[sentcount].first;
				uint8_t findid = send_find_value[sentcount].second;
				std::list<Node>::iterator itr;
				for (itr=kbuckets[bucknum].begin(); itr != kbuckets[bucknum].end(); ++itr) {
					if (itr->id == findid) {
						break;
					}
				}
				struct sockaddr_in send;
				memset(&send, 0, sizeof(send));
				send.sin_family = AF_INET;
				send.sin_port = htons(itr->port);
				inet_pton(AF_INET, (itr->name).c_str(), &(send.sin_addr));

				std::string message = "FIND_DATA " + std::to_string(lookingforkey) + "\n";

				sentcount = 1;
				waitforvalue = 1;
				if (sentcount == send_find_value.size()) {
					waitforvalue = 0;
					sendmore = 0;
				}
			}

		}

	}
}