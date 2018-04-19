
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

#include "packet.h"
#include "tracker_client.h"
#include "file_transfer.h"

#define MAXBUFSIZE 1024
#define MAXTASKSCOUNT 10

static char *DELIM = ":";

// these mutex objects should be shared between client thread and server thread somehow.
// Maybe declare them in the main function and pass them into client & server as parameter?
pthread_mutex_t tasks_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_conds[MAXTASKSCOUNT];
for (int i = 0; i < MAXTASKSCOUNT; i ++){
	task_conds[i] = PTHREAD_COND_INITIALIZER;
}
pthread_cond_t add_task_cond = PTHREAD_COND_INITIALIZER;

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

	char data_in_byte;

	
	if((read(sockfd, &data_in_byte, 1)) == -1){
		return -1;
	}
	
	int itr = 0;
	while (data_in_byte != '\n') {
		buf[itr] = data_in_byte;
		if (read(sockfd, &data_in_byte, 1) == -1)
			return -1;
		itr ++;
	}

	rv = itr; // rv records the size of packet.

	return rv;
}

// method that server.c will call
list* readPacket(int sockfd, list* head, int *tasks_count, char **tasks_name){
	char buf[MAXBUFSIZE];

	readOutPacket(sockfd, buf);

	return decodePacket(sockfd, buf, head, tasks_count, tasks_name);
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
	else if(!strcmp(header, "ASK_DL")){	// requester asks peer for download, close connection
		rv = ASK_DL;
	}
	else if(!strcmp(header, "START_SD")){	// peer tells requester that peer will start sending file, maintain connection. 
		rv = START_SD;
	}

	free(orig);

	return rv;
}

// this method will be running when receiving packets
list* decodePacket(int sockfd, char *buf, list* head, int *tasks_count, char **tasks_name){

	int packet_num = parse_packet_header(buf);

	return decodePacketNum(sockfd, buf, packet_num, head, tasks_count, tasks_name);

}


// this method focused on building packets to send
void makePacket(char *buf, char *filename, char *ip, char *port, size_t filesize, size_t index, int packet_num){
	
	switch(packet_num){
		case ASK_REQ:
			ask_reqPacket(buf, filename, ip, port);
			break;
		case RESP_REQ:
			resp_reqPacket(buf, filename, filesize, peers);
			break;
		case ASK_AVAIL:
			ask_availPacket(buf, filename);
			break;
		case RESP_AVAIL:
			resp_availPacket(buf, filename, filesize);
			break;
		case ASK_DL:
			ask_dlPacket(buf, filename, filesize, index);
			break;
		case START_SD:
			start_sdPacket(buf, filename, filesize, index);
			break;
	}
}

// this method is designed to be sending packets
int sendPacket(int sockfd, char *buf, char *filename, char *ip, char *port, size_t *filesize, int index, int packet_num){

	makePacket(buf, filename, ip, port, filesize, index, packet_num); // creates the packet according to packet_num
								 
	return sendHelper(sockfd, buf);
}

int find_tasks(char **tasks_name, char *filename) {
	int itr = 0;
	while (itr < MAXTASKSCOUNT) {
		if (strcmp(tasks_name[itr], filename) == 0) {
			return itr;
		}
		itr ++;
	}
	printf("Cannot find tasks_itr given filename: %s\n", filename);
	return -1;
}

int add_tasks(char **tasks_name, char *filename){
	int itr = 0;
	while (itr < MAXTASKSCOUNT) {
		if (tasks_name[itr] == NULL) {
			tasks_name[itr] = filename;
			return itr;
		}
	}
	printf("Cannot insert filename: %s into tasks_name, reach maximum tasks count\n", filename);
	return -1;
}


// packet_num corresponds to the kinds of packets we have defined
// this method is designed to be decoding received packets
list* decodePacketNum(int sockfd, char *buf, int packet_num, list* head, int *tasks_count, char **tasks_name){

	char *buf_copy = strdup(buf);
	char *orig = buf_copy; 				// part of cleanup
	strtok(buf_copy, DELIM); 			// now buf_copy is the data

	//declaration of statements
	char *filename; 	// filename
	char *ip; 		// ip
	char *port;		// port
	size_t filesize;	// size of the file being transferred
	size_t index; 		// index of the segment
	char *peers;		// a string containing ip:port of all other nodes containing the target file, formated as ip:port:ip:port...

	int numUsers = 0;

	switch(packet_num){
		case ASK_REQ: 
			// ASKREQ:FILENAME:IP:PORT (port being the port
			// that the requester's server is on)
			// 
			// someone wants file, i (tracker) need to ask
			// others to see if they have the files	

			filename = strtok(NULL, DELIM); // filename
			ip = strtok(NULL, DELIM); 	// ip
			port = buf_copy;		// port

			head = newConnection(head, ip, port);

			head = connectAll(head, filename, &numUsers);

			// wait for responses??
			// so I can compile a list?

			break;
		case RESP_REQ:
			// tracker has sent back a list
			// now we should download from others
			//
			// Send ASK_DL to all peers(get peer info from RESP_REQ)
			// Close connections and wait() till server wakes me up
			
			filename = strtok(NULL, DELIM);

			pthread_mutex_lock(&tasks_lock);
			int tasks_itr = find_tasks(tasks_name, filename);
			if (tasks_itr == -1)
				tasks_itr = add_tasks(tasks_name, filename);
			if (tasks_itr == -1) 
				// we reach MAXTASKSCOUNT, wait till there's a spot in tasks_name so i can add task
				pthread_cond_wait(&add_task_cond, &tasks_lock);
			pthread_mutex_unlock(&tasks_lock);

			char *filesize_string = strtok(NULL, DELIM);
			if (sscanf(filesize_string, "%zu", &filesize) == EOF) {
				perror("Failed sscanf()");
				free(args);
				return NULL;
			} 
			
			char **peers_ip = (char **)malloc(32 * sizeof(char *));
			char **peers_port = (char **) malloc(32 * sizeof(char *));

			char *token;
			int peers_itr = 0;
			while ((token = strtok(NULL, DELIM)) != NULL) {
				peers_ip[peers_itr] = token;
				token = strtok(NULL, DELIM);
				peers_port[peers_itr] = token;
				peers_itr ++;
			}
			int segment_count = peers_itr;
			size_t *segments = (size_t *)malloc(segment_count * sizeof(size_t)); // records size of each segment
			schedule_segment_size(segments, filesize, segment_count);
			
			peers_itr = 0;
			int sockfd = 0;
			while (peers_itr < segment_count) {
				char buf[MAXBUFSIZE];
				sockfd = getConnection(peers_ip[peers_itr], peers_port[peers_itr]);
				if (sockfd == -1) {
					perror("Failed getConnection()");
					continue; // try to connect to next peer
				}

				if (p_client(sockfd, peers_ip[peers_itr], peers_port[peers_itr], filename, buf, segments[peers_itr], peers_itr, ASK_DL) != 0) {
					// p_client send ASK_DL to the target peer given sockfd
					printf("Failed t_client() on ip %s, port %s\n", peers_ip[peers_itr], peers_port[peers_itr]);
				}

				if (close(sockfd) == -1) {
					perror("Failed close()");
				}

				peers_itr ++;
			}
			pthread_mutex_lock(&tasks_lock);
			// all ASK_DL packets have been sent, now wait for server to wake me up
			// condition wait on the corresponding condition variable
			while (tasks_count[tasks_itr] > 0) 
				pthread_cond_wait(&task_conds[tasks_itr], &tasks_lock);
			pthread_mutex_unlock(&tasks_lock);

			// now combine the files and resent ASK_DL for missing segments.
			// combine_file() will malloc an int* that records the indexs of the missing segments
			// the last element in this int* is set to 0
			// if no segment is missing it will return NULL
			int *missing_segments = combine_file(filename, segment_count);

			// iterate through missing_segments to get the index of missing segments
			// then ask the corresponding peers again for the same segments
			while (missing_segments != NULL) {
				int miss_itr = 0;
				// the last element is set to 0 in combine_file()
				while (missing_segments[miss_itr] != 0) {

					int miss_index = missing_segments[miss_itr];

					char buf[MAXBUFSIZE];

					sockfd = getConnection(peers_ip[miss_index], peers_port[miss_index]);
					if (sockfd == -1) {
						printf("Failed getConnection()\n");
						continue;
					}
						printf("Failed t_client() on ip %s, port %s\n", peers_ip[miss_index], peers_port[miss_index]);
					}
					if (close(sockfd) == -1) {
						perror("Failed close()");
					}

					miss_itr ++;
				}

				pthread_mutex_lock(&tasks_lock);
				while (tasks_count[tasks_itr] > 0) {
					pthread_cond_wait(&task_conds[tasks_itr], &tasks_lock);
				}
				pthread_mutex_unlock(&tasks_lock);

				free(missing_segments);
				missing_segments = combine_file(filename, segment_count);
			}
			
			
			// now we finished a task, remove the info in tasks_name and tasks_count
			// wake up the add_task_cond so other tasks can be handled.
			pthread_mutex_lock(&tasks_lock);
			tasks_name[tasks_itr] = NULL;
			// tasks_count[tasks_itr] is already 0
			pthread_cond_signal(&add_task_cond);
			pthread_mutex_unlock(&tasks_lock);
	
			free(missing_segments);
			free(peers_ip);
			free(peers_port);
			free(segments);
			break;

		case ASK_AVAIL:
			// ASK_AVAIL:FILENAME
			//
			// tracker wants to know if I have file
			// so I will check the filessystem and
			// send my info if I have or dont have
			//
			// Call get_filesize(filename), which tries
			// to open the file and return its size (-1 
			// on failure). Then send a RESP_AVAIL packet
			// containing the filesize info back to tracker
			//filename = strtok(NULL, DELIM);
			//size_t filesize = get_filesize(filename); // get_filesize is in file_transfer.c
			//					  // it returns size of the file on success, -1 on failure

			//char buf[MAXBUFSIZE];
			//if (sendPacket(sockfd, buf, filename, NULL, NULL, filesize, 0, RESP_AVAIL) < 0) {
			//	printf("Fail sendPacket() for file named %s, packet type %s\n", filename, "RESP_AVAIL");
			//	return NULL;
			//}

			break;
		case RESP_AVAIL:
			// someone has sent me back their file
			// availability, lets compile a list
			// linked list?
			//
			// If filesize == -1, then the packet sender
			// does not have the file.
			// Otherwise, add the packet sender's ip and port
			// to a char *peers, and send the string to the requester.
			// the string should be formatted as ip:port:ip:port
			//filename = strtok(NULL, DELIM);
			//filesize_string = strtok(NULL, DELIM);
			//size_t filesize = 0;
			//if (sscanf(filesize_string, "%zu", &filesize) == EOF) {
			//	printf("Failed sscanf() filesize_string %s\n", filesize_string);
			//	return NULL;
			//} 

				
		

			break;
		case ASK_DL:
			// a requester has asked to download from me
			//
			// Call p_client to send a START_SD packet to the requester
			// Call send_file() to send the file, which will return -1 on failure.
			filename = strtok(NULL, DELIM);

			char *filesize_string = strtok(NULL, DELIM);
			if (sscanf(filesize_string, "%zu", &filesize) == EOF) {
				perror("Failed sscanf()");
				free(args);
				return NULL;
			} 

			char *index_string = strtok(NULL, DELIM);
			if (sscanf(index_string, "%zu", &index) == EOF) {
				perror("Failed sscanf()");
				free(args);
				return NULL;
			} 

			int sockfd = getConnection(ip, port);
			if (sockfd == -1) {
				printf("Failed getConnection()");
				free(args);
				return NULL;
				
			}

			char buf[MAXBUFSIZE];
			if (p_client(sockfd, ip, port, filename, buf, filesize, index, START_SD) == -1) {
				printf("Failed p_client() for file named %s, index %d\n", filename, index);
				free(args);
				return NULL;
			}


			if (send_file(filename, sockfd, index, filesize) < 0) {
				printf("Failed send_file() for file named %s, index %d\n", filename, index);
				free(args);
				return NULL;
			}

			if (close(sockfd) == -1) {
				perror("Failed close()");
				free(args);
				return NULL;
			}

			break;

		case START_SD:
			filename = strtok(NULL, DELIM);
			char *filesize_string = strtok(NULL, DELIM);
			if (sscanf(filesize_string, "%zu", &filesize) == EOF) {
				perror("Failed sscanf()");
				free(args);
				return NULL;
			} 

			char *index_string = strtok(NULL, DELIM);
			if (sscanf(index_string, "%zu", &index) == EOF) {
				perror("Failed sscanf()");
				free(args);
				return NULL;
			} 

			if (recv_file(filename, sockfd, index, filesize) != filesize) {
				printf("Failed recv_file()\n");
				free(args);
				return NULL;
			}
			pthread_mutex_lock(&tasks_lock);
			int tasks_itr = find_tasks(tasks_name, filename);
			if (tasks_itr == -1) {
				printf("Failed find_tasks(): task is not in the tasks array");
				pthread_mutex_unlock(&tasks_lock);
				break;
			}
			tasks_count[tasks_itr] --;
			if (tasks_count[tasks_itr] == 0) {
				pthread_cond_broadcast(&tasks_cond[tasks_itr]);
			}
			pthread_mutex_unlock(&tasks_lock);

			break;
	}
	free(orig);
	return head;

}

// Below here is the list of packets that can be 'maked'

int ask_reqPacket(char *buf, char *filename, char *ip, char *port){

	char *header = "ASK_REQ";

	sprintf(buf, "%s:%s:%s:%s", header, filename, ip, port);

	buf[strlen(buf)] = '\n';

	return 0;
}

int resp_reqPacket(char *buf, char *filename, size_t filesize){
	// silence warnings;
	if(buf == filename)
		return 1;
	return 0;
}

int ask_availPacket(char *buf, char *filename){
	// @para filename: name of the target file
	// 	ip, port: ip and port of the tracker
	char *header = "ASK_AVAIL";

	sprintf(buf, "%s:%s", header, filename);
	
	buf[strlen(buf)] = '\n';

	return 0;
}

int resp_availPacket(char *buf, char *filename){
	// @para filename: name of the target file
	// 	filesize: size of the entire target file (instead of a segment of the target file)
	char *header = "RESP_AVAIL";

	sprintf(buf, "%s:%s", header, filename);
	
	buf[strlen(buf)] = '\n';

	return 0;
}

int ask_dlPacket(char *buf, char *filename, size_t filesize, size_t index){
	// @para filename: name of the target file
	//
	// 	filesize: size of the segment
	//
	// 	index: index of the segment. 
	// 	if (index == 0), send the entire file with no partitioning
	// 	if (index > 0), send the corresponding segment of index
	char *header = "ASK_DL";

	sprintf(buf, "%s:%s:%zu:%zu", header, filename, filesize, index);
	
	buf[strlen(buf)] = '\n';

	return 0;
}

int start_sdPacket(char *buf, char *filename, size_t filesize, size_t index){
	char *header = "START_DL";

	sprintf(buf, "%s:%s:%zu:%zu", header, filename, filesize, index);
	
	buf[strlen(buf)] = '\n';

	return 0;
}
