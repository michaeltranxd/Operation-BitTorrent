
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "packet.h"
#include "tracker_client.h"

#define MAXBUFSIZE 1024

static char *DELIM = ":";

// connects to every node in network
list* connectAll(list* head, char *filename, int* numConnections){
	if(head == NULL)
		return head;

	list* curr = head;
	list* next;

	while(curr != NULL){
		next = curr->next;

		if(connectAndSend(curr, filename) == -1)// failed connection so we remove	
			head = removeConnection(head, curr);
		else{
			(*numConnections)++;
		}

		curr = next;
	}
	return head;
}

// helper function that does the connect and send packet
int connectAndSend(list* node, char *filename){
	char buf[MAXBUFSIZE];

	if(t_client(node->ip, node->port, filename, buf, ASK_AVAIL) != 0) // meaning we have failed
		return -1;
	return 0;
}

// method that checks if connection is a new connection
list* newConnection(list* head, char *ip, char *port){
	list* new_list = malloc(sizeof(list));
	new_list->ip = ip;
	new_list->port = ip;
	new_list->next = NULL;
	if(head == NULL){ // new element should be head;
		return new_list;
	}

	list* node = findConnection(head, ip);

	if(node == NULL){ // did not find it so we add
		addConnection(head, new_list);
	}
	// found it so we dont need to do anything
	return head;
}

// adds the connection
void addConnection(list* head, list* new_list){
	list* curr = head;
	while(curr->next != NULL){
		curr = curr->next;
	}
	curr->next = new_list;
}

// removes the connection
list* removeConnection(list* head, list* remove){ // should only be called when connection failed with other nodes
	list* node = remove;

	if(head == node){
		free(node);
		return head->next;
	}

	list* curr = head;
	while(curr->next != node){
		if(curr == NULL){ // never found it
			return NULL;
		}
		curr = curr->next;
	}
	curr->next = node->next;			// --------    --------    --------
	free(node);							// |	  |    |      |    |      |
	return head;						// | curr |--->| node |--->| next |
										// |      |    |      |    |      |
										// --------    --------    --------
										//
										//               to
										//
										//       --------    --------
										//       |      |    |      |
										//       | curr |--->| next |
										//       |      |    |      |
										//       --------    --------	
	
	
}

// helper function that finds connection
list* findConnection(list* head, char *ip){
	list* curr = head;

	if(curr == NULL)
		return NULL;

	while(curr->next != NULL){
		if(!strcmp(curr->ip, ip)){
			return curr;
		}
		curr = curr->next;
	}

	return NULL;
}

// cleans up the list
void destroyList(list* head){

	list* curr = head;
	list* old;

	while(curr != NULL){
		old = curr;
		curr = curr->next;

		free(old);
	}
	
}

// IGNORE THIS!! (this will be useful when implementing
// server.c when we get there)
int serverHelper(int sockfd, char *hostname){

	char buf[1024];

	if(readOutPacket(sockfd, buf) == -1){
		return -1;
	}

	decodePacket(buf, NULL);

	return 0;
}

// IGNORE THIS!! (this will be useful when implementing
// client.c when we get there)
int clientHelper(int sockfd, char *filename, char *myip, char *myserverport){

	char buf[strlen(filename) + strlen("ASK_REQ:") + 1]; // ASK:FILENAME_

	sendPacket(sockfd, buf, filename, myip, myserverport, ASK_REQ);

	return 1;

}

// Helper function which simply writes it on the socket
int sendHelper(int sockfd, char *packet){
	int rv = 0;	

	if((rv = write(sockfd, packet, strlen(packet))) == -1){
		return -1;
	}

	return rv;

}

// function that simply reads out the packet
int readOutPacket(int sockfd, char *buf){
	int rv = 0;

	if((rv = read(sockfd, buf, MAXBUFSIZE)) == -1){
		return -1;
	}

	return rv;
}

// method that server.c will call
list* readPacket(int sockfd, list* head){
	char buf[MAXBUFSIZE];

	readOutPacket(sockfd, buf);

	return decodePacket(buf, head);
}

// just parsing to find the packet number
int parse_packet_header(char *buf){

	char *buf_copy = strdup(buf);
	char *orig = buf_copy; 					// part of cleanup
	char *header = strtok(buf_copy, DELIM);

	int rv = -1;

	if(!strcmp(header, "ASK_REQ")){
		rv = ASK_REQ;
	}
	else if(!strcmp(header, "RESP_REQ")){
		rv = RESP_REQ;
	}
	else if(!strcmp(header, "ASK_AVAIL")){
		rv = ASK_AVAIL;
	}
	else if(!strcmp(header, "RESP_AVAIL")){
		rv = RESP_AVAIL;
	}
	else if(!strcmp(header, "ASK_DL")){
		rv = ASK_DL;
	}
	else if(!strcmp(header, "RESP_DL")){
		rv = RESP_DL;
	}

	free(orig);

	return rv;
}

// this method will be running when receiving packets
list* decodePacket(char *buf, list* head){

	int packet_num = parse_packet_header(buf);

	return decodePacketNum(buf, packet_num, head);

}


// this method focused on building packets to send
void makePacket(char *buf, char *filename, char *ip, char *port, int packet_num){
	
	switch(packet_num){
		case ASK_REQ:
			ask_reqPacket(buf, filename, ip, port);
			break;
		case RESP_REQ:
			resp_reqPacket(buf, filename);
			break;
		case ASK_AVAIL:
			ask_availPacket(buf, filename);
			break;
		case RESP_AVAIL:
			resp_availPacket(buf, filename);
			break;
		case ASK_DL:
			ask_dlPacket(buf, filename);
			break;
		case RESP_DL:
			resp_dlPacket(buf, filename);
			break;
	}
}

// this method is designed to be sending packets
int sendPacket(int sockfd, char *buf, char *filename, char *ip, char *port, int packet_num){

	makePacket(buf, filename, ip, port, packet_num); // creates the packet according to
								 // the packet_num
	return sendHelper(sockfd, buf);
}

// packet_num corresponds to the kinds of packets we have defined
// this method is designed to be decoding receiving packets
list* decodePacketNum(char *buf, int packet_num, list* head){

	char *buf_copy = strdup(buf);
	char *orig = buf_copy; 				// part of cleanup
	strtok(buf_copy, DELIM); 			// now buf_copy is the data

	//declaration of statements
	char *filename; 	// filename
	char *ip; 		// ip
	char *port;		// port
	char *filesize;		// size of the file being transferred
	char *index; 		// index of the segment

	int numUsers = 0;

	switch(packet_num){
		case ASK_REQ: // ASKREQ:FILENAME:IP:PORT (port being the port
					  // that the requester's server is on

			// someone wants file, we need to ask
			// others to see if they have the files	

			filename = strtok(NULL, DELIM); // filename
			ip = strtok(NULL, DELIM); // ip
			port = buf_copy;			  // port

			head = newConnection(head, ip, port);

			head = connectAll(head, filename, &numUsers);

			// wait for responses??
			// so I can compile a list?

			break;
		case RESP_REQ:
			// tracker has sent back a list
			// now we should download from others
			break;
		case ASK_AVAIL:
			// tracker wants to know if I have file
			// so I will check the filessystem and
			// send my info if I have or dont have
			filename = strtok(NULL, DELIM);
			ip = strtok(NULL, DELIM); 
			port = buf_copy;			  

			break;
		case RESP_AVAIL:
			// someone has sent me back their file
			// availability, lets compile a list
			// linked list?
			filename = strtok(NULL, DELIM);
			filesize = strtok(NULL, DELIM);

			break;
		case ASK_DL:
			// my peer has asked to download
			filename = strtok(NULL, DELIM);
			filesize = strtok(NULL, DELIM);
			index = strtok(NULL, DELIM);


			break;
		case RESP_DL:
			// first check if has file, then if it does
			// send file
			filename = strtok(NULL, DELIM);
			filesize = strtok(NULL, DELIM);
			index = strtok(NULL, DELIM);
			break;
	}
	free(orig);
	return head;

}

// Below here is the list of packets that can be 'maked'

int ask_reqPacket(char *buf, char *filename, char *ip, char *port){

	char *header = "ASK_REQ";

	sprintf(buf, "%s:%s:%s:%s", header, filename, ip, port);

	buf[strlen(buf)] = '\0';

	return 0;
}

int resp_reqPacket(char *buf, char *filename){
	// silence warnings;
	if(buf == filename)
		return 1;
	return 0;
}

int ask_availPacket(char *buf, char *filename, char *ip, char *port){
	// @para filename: name of the target file
	// 	ip, port: ip and port of the tracker
	char *header = "ASK_AVAIL";

	sprintf(buf, "%s:%s:%s:%s", header, filename, ip, port);
	
	buf[strlen(buf)] = '\0';

	return 0;
}

int resp_availPacket(char *buf, char *filename, char *filesize){
	// @para filename: name of the target file
	// 	filesize: size of the entire target file (instead of a segment of the target file)
	char *header = "RESP_AVAIL";

	sprintf(buf, "%s:%s:%s", header, filename, filesize);
	
	buf[strlen(buf)] = '\0';

	return 0;
}

int ask_dlPacket(char *buf, char *filename, char *filesize, char *index){
	// @para filename: name of the target file
	//
	// 	filesize: size of the segment
	//
	// 	index: index of the segment. 
	// 	if (index == 0), send the entire file with no partitioning
	// 	if (index > 0), send the corresponding segment of index
	char *header = "ASK_DL";

	sprintf(buf, "%s:%s:%s:%s", header, filename, filesize, index);
	
	buf[strlen(buf)] = '\0';

	return 0;
}

int resp_dlPacket(char *buf, char *filename, char *filesize, char *index){
	char *header = "RESP_DL";

	sprintf(buf, "%s:%s:%s%s", header, filename, filesize, index);
	
	buf[strlen(buf)] = '\0';

	return 0;
}
