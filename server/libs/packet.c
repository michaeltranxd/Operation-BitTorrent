
#include "packet.h"
#include "../peer.h"

#define MAXBUFSIZE 1024
#define MAXTASKSCOUNT 10
#define MAXPEERSCOUNT 8

static char *DELIM = ":";


char **tasks_name;
int *tasks_count;
pthread_mutex_t task_lock;
pthread_cond_t task_conds[MAXTASKSCOUNT];
pthread_cond_t add_task_cond;
// these mutex objects should be shared between client thread and server thread somehow.
// Maybe declare them in the main function and pass them into client & server as parameter?

/*
*
* TRACKER
*
*/

// connects to every node in network
list* connectAll(list* head, char* filename, int* numConnections, char* buf, char* ip){
	if(head == NULL)
		return head;

	list* curr = head;
	list* next;

	long long rv;

	while(curr != NULL){
		next = curr->next;

		printf("connectAll check\n");
		if(!strcmp(curr->ip, ip)){
			curr = next;
			continue;
		}
		printf("not requester\n");

		rv = connectAndSend(curr, filename);

		if (rv > 0) {
			(*numConnections)++;

			if (*numConnections == 1) { // first connection, add filesize to buf
				char filesize[256];
				sprintf(filesize, ":%lld", rv);
				strcat(buf, filesize);
		}


			printf("Before add:%s\n", buf);

			char str[256];
			memset(str, 0, 256);
			sprintf(str, ":%s:%s", curr->ip, curr->port);
			strcat(buf, str);

			printf("this is str:%s\n", str);

			printf("After add:%s\n", buf);

			// append good ips to buf
		}
		else if (rv == -1) // failed connection so we remove
			head = removeConnection(head, curr);

		curr = next;
	}

	strcat(buf, "\n");

	return head;
}

// helper function that does the connect and send packet
long long connectAndSend(list* node, char* filename){
	char buf[MAXBUFSIZE];
	memset(buf, 0, MAXBUFSIZE);

	printf("connecting to peer at %s %s\n", node->ip, node->port);

	long long rv = client(node->ip, node->port, NULL, filename, buf, 0, 0, ASK_AVAIL);

	printf("connectAndSend rv is: %lld\n", rv);

	if (rv >= 0) // rv is filesize if node has file, else node would return 0
		return rv;
	else // failed to connect to node
		// or filesize is 0
		return -1;
}

// method that checks if connection is a new connection
list* newConnection(list* head, char *ip, char *port){
	list* new_list = malloc(sizeof(list));
	new_list->ip = strdup(ip);
	new_list->port = strdup(port);
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

	while(curr != NULL){
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

// Helper function which simply writes it on the socket
int sendHelper(int sockfd, char *packet){
	int rv = 0;	

	if((rv = write(sockfd, packet, strlen(packet))) == -1){
		return -1;
	}

	printf("rv:%d sent!\n", rv);

	return rv;

}

// function that simply reads out the packet
int readOutPacket(int sockfd, char *buf){
	int rv = 0;

	char data_in_byte;

	memset(buf, 0, MAXBUFSIZE);

	printf("Starting readOutPacket()\n");
	printf("socket readOutPacket:%d\n", sockfd);

	if((read(sockfd, &data_in_byte, 1)) == -1){
		perror("failed!");
		return -1;
	}

	printf("Recveived first byte\n");

	
	int itr = 0;
	while (data_in_byte != '\n') {
		printf("char: %c\n", data_in_byte);
		buf[itr] = data_in_byte;
		if (read(sockfd, &data_in_byte, 1) == -1)
			return -1;
		itr ++;
	}

	printf("Content of the packet is %s\n", buf);

	rv = itr; // rv records the size of packet.



	return rv;
}

// method that server.c will call
list* readPacket(int sockfd, list* head, char *req_ip, int *tasks_count, char **tasks_name){
	char buf[MAXBUFSIZE];
	memset(buf, 0, MAXBUFSIZE);

	readOutPacket(sockfd, buf);

	list* rv = decodePacket(sockfd, buf, head, req_ip, tasks_count, tasks_name);

//	shutdown(sockfd, 0); // read shutdowned

	write(sockfd, "OK\n", 3);

	return rv;
}

// just parsing to find the packet number
int parse_packet_header(char *buf){

	char *buf_copy = strdup(buf);
	char *orig = buf_copy; 					// part of cleanup
	char *header = strtok(buf_copy, DELIM);

	int rv = -1;

	if(!strcmp(header, "ASK_REQ")){
		printf("TRACKER GOT PACKET!!!\n");
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
list* decodePacket(int dl_sockfd, char *buf, list* head, char *req_ip, int *tasks_count, char **tasks_name){

	int packet_num = parse_packet_header(buf);

	return decodePacketNum(dl_sockfd, buf, packet_num, head, req_ip, tasks_count, tasks_name);

}


// this method focused on building packets to send
void makePacket(char *buf, char *filename, char *ip, char *port, size_t filesize, int index, int packet_num){
	

	memset(buf, 0, MAXBUFSIZE);
	switch(packet_num){
		case ASK_REQ:
			ask_reqPacket(buf, filename, port);
			printf("ASK_REQ packet was made as: %s\n", buf);
			break;
		case RESP_REQ:
			resp_reqPacket(buf, filename, ip, port);
			printf("RESP_REQ packet was made as: %s\n", buf);
			break;
		case ASK_AVAIL:
			ask_availPacket(buf, filename);
			printf("ASK_AVAIL packet was made as: %s\n", buf);
			break;
		case RESP_AVAIL:
			resp_availPacket(buf, filename, filesize);
			printf("RESP_AVAIL packet was made as: %s\n", buf);
			break;
		case ASK_DL:
			ask_dlPacket(buf, filename, filesize, index, ip, port);
			printf("ASK_DL packet was made as: %s\n", buf);
			break;
		case START_SD:
			start_sdPacket(buf, filename, filesize, index);
			printf("START_SD packet was made as: %s\n", buf);
			break;
	}
}

// this method is designed to be sending packets
long long sendPacket(int sockfd, char* buf, char* filename, char* ip, char* port, size_t filesize, int index, int packet_num){

	makePacket(buf, filename, ip, port, filesize, index, packet_num); // creates the packet according to
								 // the packet_num

	long long rv = sendHelper(sockfd, buf);

	// read RESP_AVAIL packet from node
	if (packet_num == ASK_AVAIL) {
		char resp_buf[MAXBUFSIZE];

		readOutPacket(sockfd, resp_buf);

		strtok(resp_buf, DELIM); // header
		strtok(NULL, DELIM);	 // filename
		char* char_fs = strtok(NULL, DELIM); // filesize

		printf("resp_buf: %s\n", char_fs);

		long long filesize = atoll(char_fs);	// buf_copy contains str file_size
		// <= 0 if file not there, else filesize
		printf("atoll returned filesize: %llu\n", filesize);

		if(filesize < 0){
			close(sockfd);
			return -1;
		}

		// if packet is yes
		return filesize;
	}
	else if (packet_num == ASK_DL) {

	}

	return rv;
}

static void print_tasks_name() {
	int i = 0;
	for (; i < MAXTASKSCOUNT; i ++) {
		printf("task name at tasks_name[%d] is: %s\n", i, tasks_name[i]);
	}
}

static int find_task(char *filename) {
	int itr = 0;
	printf("(find_task) filename is %s\n", filename);
	//print_tasks_name();
	for (; itr < MAXTASKSCOUNT; itr ++) {
		//printf("(find_task)looking at tasks_name[%d]\n", itr);
		//printf("(find_task)looking at tasks_name: %s\n", tasks_name[itr]);
		if (tasks_name[itr] == NULL){}
		else if (strcmp(tasks_name[itr], filename) == 0) {
			return itr;
		}
	}
	printf("Cannot find tasks_itr given filename: %s\n", filename);
	return -1;
}

static int add_task(char *filename, size_t segment_count){
	int itr = 0;
	printf("(add_task) filename is %s\n", filename);
	while (itr < MAXTASKSCOUNT) {
		//printf("(add_task) looking at tasks_name[%d]\n", itr);
		if (tasks_name[itr] == NULL) {
			//tasks_name[itr] = strndup(filename, strlen(filename));
			tasks_name[itr] = filename;
			tasks_count[itr] = segment_count;
			printf("return itr %d %s\n", itr, tasks_name[itr]);
			return itr;
		}
		itr ++;
	}
	printf("Cannot add filename:%s into tasks_name, reach MAXTASKSCOUNT\n", filename);
	return -1;
}


// packet_num corresponds to the kinds of packets we have defined
// this method is designed to be decoding received packets
list* decodePacketNum(int dl_sockfd, char *buf, int packet_num, list* head, char *req_ip, int *tasks_count, char **tasks_name){

	char *buf_copy = strdup(buf);
	char *orig = buf_copy; 				// part of cleanup
	strtok(buf_copy, DELIM); 			// now buf_copy is the data

	//declaration of statements
	char *filename; 	// filename
	char *ip; 		// ip
	char *port;		// port
	size_t filesize;	// size of the file being transferred
	int index; 		// index of the segment
//	char *peers;		// a string containing ip:port of all other nodes containing the target file, formated as ip:port:ip:port...

	int numUsers = 0;

	// declaring variables to avoid compiling issues
	int sockfd;
	int tasks_itr;
	char *filesize_string;

	switch(packet_num){
		case ASK_REQ: 
			// ASKREQ:FILENAME:IP:PORT (port being the port
			// that the requester's server is on)
			// 
			// someone wants file, i (tracker) need to ask
			// others to see if they have the files	

			filename = strtok(NULL, DELIM); // filename
			ip = req_ip; 	// ip
			port = strtok(NULL, DELIM);	// port

			printf("(received ASK_REQ) Start newConnection() to ip:%s port:%s\n", ip, port);

			head = newConnection(head, ip, port);

			printf("(received ASK_REQ) PRINTING LIST!\n");
			list* curr = head;
			while(curr != NULL){
				printf("%s\n", curr->ip);
				printf("%s\n", curr->port);
				curr = curr->next;
			}

			makePacket(buf, filename, ip, port, 0, 0, RESP_REQ);

			printf("(received ASK_REQ) Start connectAll() \n");
			head = connectAll(head, filename, &numUsers, buf, ip);

			if (numUsers != 0) {
				printf("(received ASK_REQ) connecting back to requester!\n");
				if ((sockfd = getConnection(ip, port)) == -1){ // failed
					break;
				}

				sendHelper(sockfd, buf);

				printf("%s\n", buf);

				printf("(received ASK_REQ) sent packet to requester RESP_REQ\n");

				close(sockfd);
			}
			else { // no one has file
				printf("(received ASK_REQ) no one has file haha\n");
				// send error messgae back to requester

			}

			break;
		case RESP_REQ:
			// tracker has sent back a list
			// now we should download from others
			//
			// Send ASK_DL to all peers(get peer info from RESP_REQ)
			// Close connections and wait() till server wakes me up
			
			filename = strtok(NULL, DELIM);

			ip = strtok(NULL, DELIM);
			
			port = strtok(NULL, DELIM);

			filesize_string = strtok(NULL, DELIM);
			if (sscanf(filesize_string, "%zu", &filesize) == EOF) {
				perror("Failed sscanf()");
//				free(args);
				return NULL;
			} 
			printf("(received RESP_REQ) filename:%s, my ip:%s, my port:%s\n", filename, ip, port);
			

			// parse the rest of the RESP_REQ packet to get ip, port of peers, and segment_count.
			char **peers_ip = (char **)malloc(MAXPEERSCOUNT * sizeof(char *));
			char **peers_port = (char **) malloc(MAXPEERSCOUNT * sizeof(char *));
			printf("finished malloc peers_ip and peers_port\n");
			char *token;
			int peers_itr = 0;
			while ((token = strtok(NULL, DELIM)) != NULL) {
				printf("now initialize peers_ip[%d] with %s\n", peers_itr, token);
				peers_ip[peers_itr] = token;
				token = strtok(NULL, DELIM);
				printf("now initialize peers_port[%d] with %s\n", peers_itr, token);
				peers_port[peers_itr] = token;
				peers_itr ++;
			}
			int segment_count = peers_itr;
			size_t *segments = (size_t *)malloc(segment_count * sizeof(size_t)); // records size of each segment
			schedule_segment_size(segments, filesize, segment_count);
			if (segments[0] == filesize) // if the filesize is less than two pages, schedule_segment_size() will initialize segments[0] with filesize, and the requester should only ask one node about the file.
				segment_count = 1;

			// initialize corresponding tasks_name[] and tasks_count[]
			pthread_mutex_lock(&task_lock);
			printf("(received RESP_REQ) Start print_tasks_name() and find_task()\n");
			print_tasks_name();
			tasks_itr = find_task(filename);
			if (tasks_itr == -1)
				printf("(received RESP_REQ) find_task() returns -1, try add_task()");
				
			while ((tasks_itr = add_task(filename, segment_count)) == -1) {
				printf("(received RESP_REQ) add_task() returns -1, wait till theres a spot in tasks_name and add_task() again\n");
				pthread_cond_wait(&add_task_cond, &task_lock);
			}
			printf("(received RESP_REQ) either find_task(%s) succeed or add_task(%s) succeed, now proceed\n", filename, filename);
			pthread_mutex_unlock(&task_lock);


			peers_itr = 0;
			int peer_fd = 0;
			while (peers_itr < segment_count) {
				char buf[MAXBUFSIZE];
				peer_fd = getConnection(peers_ip[peers_itr], peers_port[peers_itr]);
				if (peer_fd == -1) {
					perror("Failed getConnection()");
					peers_itr ++;
					continue; // try to connect to next peer
				}

				// we need our own ip/port
				// send directly instead of asking p_client
				if (segment_count == 1) {
					if (sendPacket(peer_fd, buf, filename, ip, port, segments[peers_itr], 0, ASK_DL) == -1) {
						perror("Failed sendPacket()\n");
					} 
				}
				else {
					if (sendPacket(peer_fd, buf, filename, ip, port, segments[peers_itr], peers_itr + 1, ASK_DL) == -1) {
						perror("Failed sendPacket()\n");
					} 
				}

				printf("Finished sendPacket() to the %dth peer\n", peers_itr);
				if (close(peer_fd) == -1) {
					perror("Failed close()");
				}

				printf("Finished close() the socket related to the %dth peer\n", peers_itr);

				peers_itr ++;
			}
			printf("Finished sendPacket() to all available peers\n");
			pthread_mutex_lock(&task_lock);
			// all ASK_DL packets have been sent, now wait for server to wake me up
			// condition wait on the corresponding condition variable
			while (tasks_count[tasks_itr] > 0)
				pthread_cond_wait(&task_conds[tasks_itr], &task_lock);
			pthread_mutex_unlock(&task_lock);

			// now combine the files and resent ASK_DL for missing segments.
			// combine_file() will malloc an int* that records the indexs of the missing segments
			// the last element in this int* is set to 0
			// if no segment is missing it will return NULL
			int *missing_segments = combine_file(filename, segment_count);

			printf("Finished combine_file() for the first time\n");

			// iterate through missing_segments to get the index of missing segments
			// then ask the corresponding peers again for the same segments
			while (missing_segments != NULL) {
				printf("missing_segments not NULL, do resend now\n");
				int miss_itr = 1;
				int miss_count = missing_segments[0];

				pthread_mutex_lock(&task_lock);
				tasks_count[tasks_itr] = miss_count;
				pthread_mutex_unlock(&task_lock);

				// the last element is set to 0 in combine_file()
				while (missing_segments[miss_itr] != 0) {

					int miss_index = missing_segments[miss_itr];

//					char buf[MAXBUFSIZE];

					peer_fd = getConnection(peers_ip[miss_index], peers_port[miss_index]);
					if (peer_fd == -1) {
						printf("Failed getConnection()\n");
						continue;
					}

					if (sendPacket(peer_fd, buf, filename, ip, port, segments[peers_itr], missing_segments[miss_itr], ASK_DL) == -1) {
						printf("Failed sendPacket()\n");
					}

					if (close(peer_fd) == -1) {
						perror("Failed close()");
					}

					miss_itr ++;
				}

				pthread_mutex_lock(&task_lock);
				while (tasks_count[tasks_itr] > 0) {
					pthread_cond_wait(&task_conds[peers_itr], &task_lock);
				}
				pthread_mutex_unlock(&task_lock);

				free(missing_segments);
				printf("Start combine_file() for missing segments\n");
				missing_segments = combine_file(filename, segment_count);
				printf("Finish combine_file() for missing segments\n");
			
			}
			pthread_mutex_lock(&task_lock);
			tasks_name[tasks_itr] = NULL;
			// tasks_count[tasks_itr] is already 0
			pthread_cond_signal(&add_task_cond); // wake up one thread
			pthread_mutex_unlock(&task_lock);


			free(missing_segments);
			free(peers_ip);
			free(peers_port);
			free(segments);
			break;

			// work on this function ASK_AVAIL:FILENAME:FILESIZE
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

			filename = strtok(NULL, DELIM); // filename

			size_t filesize = 0;

			int file_fd = open(filename, O_RDONLY);
			if (file_fd == -1) {
				perror("Failed open()");
			}
			else{

				struct stat s;
				if (fstat(file_fd, &s) == -1) {
					perror("Failed fstat()");
					exit(-1);
				}
				filesize = s.st_size;

				close(file_fd);
				
			}

			memset(buf, 0, MAXBUFSIZE);
			sendPacket(dl_sockfd, buf, filename, NULL, NULL, filesize, 0, RESP_AVAIL);

			break;
		case RESP_AVAIL:	// RESP_AVAIL:FILESIZE
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

			// ASK_DL:FILENAME:filesize:index:IP:PORT);

			char *filesize_string = strtok(NULL, DELIM);
			if (sscanf(filesize_string, "%zu", &filesize) == EOF) {
				perror("Failed sscanf()");
//				free(args);
				return NULL;
			} 

			char *index_string = strtok(NULL, DELIM);
			if (sscanf(index_string, "%d", &index) == EOF) {
				perror("Failed sscanf()");
//				free(args);
				return NULL;
			} 

			ip = strtok(NULL, DELIM);

			port = strtok(NULL, DELIM);
				
			int sockfd = getConnection(ip, port);
			if (sockfd == -1) {
				printf("Failed getConnection()");
//				free(args);
				return NULL;
				
			}

//			char buf[MAXBUFSIZE];
//
//			whenever you are using get connection, just use send packet
//

			if (sendPacket(sockfd, buf, filename, ip, port, filesize, index, START_SD) == -1){
				printf("(received ASK_DL) Failed sendPacket() for file named %s, index %d\n", filename, index);
				return NULL;
			}
			
			send_file(filename, sockfd, index, filesize);
//			if (send_file(filename, sockfd, index, filesize) != filesize) {
//				printf("Failed send_file() for file named %s, index %d\n", filename, index);
//				free(args);
//				return NULL;
//			}

			if (close(sockfd) == -1) {
				perror("Failed close()");
//				free(args);
				return NULL;
			}

			break;

		case START_SD:
			filename = strtok(NULL, DELIM);
			filesize_string = strtok(NULL, DELIM);
			if (sscanf(filesize_string, "%zu", &filesize) == EOF) {
				perror("Failed sscanf()");
//				free(args);
				return NULL;
			} 

			index_string = strtok(NULL, DELIM);
			if (sscanf(index_string, "%d", &index) == EOF) {
				perror("Failed sscanf()");
//				free(args);
				return NULL;
			} 

			printf("(received START_SD) Start recv_file()\n");
			if (recv_file(filename, dl_sockfd, index, filesize) != filesize) {
				printf("Failed recv_file()\n");
//				free(args);
				return NULL;
			}
			printf("(received START_SD) Start mutex_lock() and find_task()\n");
			pthread_mutex_lock(&task_lock);
			tasks_itr = find_task(filename);
			if (tasks_itr == -1) {
				printf("(received START_SD) Failed find_task(): task is not in the tasks array");
				pthread_mutex_unlock(&task_lock);
				break;
			}
			tasks_count[tasks_itr] --;
			if (tasks_count[tasks_itr] == 0) {
				printf("(received START_SD) tasks_count[%d] reaches 0, now wake up client thread\n", tasks_itr);
				pthread_cond_broadcast(&task_conds[tasks_itr]);
			}
			pthread_mutex_unlock(&task_lock);

			break;
	}
	free(orig);
	return head;

}

// Below here is the list of packets that can be 'maked'

int ask_reqPacket(char *buf, char *filename, char *port){

	char* header = "ASK_REQ";

	sprintf(buf, "%s:%s:%s", header, filename, port);


	printf("made ask_req packet:%s\n", buf);

	buf[strlen(buf)] = '\n';

	return 0;
}

int resp_reqPacket(char* buf, char* filename, char* req_ip, char* req_port){
	char* header = "RESP_REQ";

	sprintf(buf, "%s:%s:%s:%s", header, filename, req_ip, req_port);

//	buf[strlen(buf)] = '\n';

	printf("RESP_REQ PACKET:%s\n", buf);

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

int resp_availPacket(char *buf, char *filename, size_t filesize){
	// @para filename: name of the target file
	// 	filesize: size of the entire target file (instead of a segment of the target file)
	char *header = "RESP_AVAIL";

	sprintf(buf, "%s:%s:%zu", header, filename, filesize);
	
	buf[strlen(buf)] = '\n';

	return 0;
}

int ask_dlPacket(char *buf, char *filename, size_t filesize, size_t index, char *ip, char *port){
	// @para filename: name of the target file
	//
	// 	filesize: size of the segment
	//
	// 	index: index of the segment. 
	// 	if (index == 0), send the entire file with no partitioning
	// 	if (index > 0), send the corresponding segment of index
	char *header = "ASK_DL";

	sprintf(buf, "%s:%s:%zu:%zu:%s:%s", header, filename, filesize, index, ip, port);
	
	buf[strlen(buf)] = '\n';

	return 0;
}

int start_sdPacket(char *buf, char *filename, size_t filesize, size_t index){
	char *header = "START_SD";

	sprintf(buf, "%s:%s:%zu:%zu", header, filename, filesize, index);
	
	buf[strlen(buf)] = '\n';

	return 0;
}

