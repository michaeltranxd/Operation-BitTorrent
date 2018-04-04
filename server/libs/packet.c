
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "packet.h"

#define MAXBUFSIZE 1024


void pushPacket(list* list, packets* node){
	pthread_mutex_lock(&(list->mutex));
	if(list->head == NULL || list->tail == NULL){
		list->head = node;
		list->tail = node;
	}
	else{
		list->tail->next = node;
		list->tail = node;
	}
	pthread_cond_signal(&(list->cv));
	pthread_mutex_unlock(&(list->mutex));
}

packets* pullPacket(list* list){
	packets* packet = NULL;
	pthread_mutex_lock(&(list->mutex));
	if(list->head != NULL){
		packet = list->head;
		list->head = list->head->next;
		if(list->head == NULL){ // means that was the last element
			list->tail = NULL;
		}
	}
	pthread_mutex_unlock(&(list->mutex));
	return packet;
}

packets* createPacket(char* packet_string){
	packets* packet = malloc(sizeof(struct packets));
	packet->packet_string = packet_string;
	packet->next = NULL;

	return packet;
}

void destroyPacket(packets* packet){
	free(packet->packet_string);
	free(packet);
}

list* createList(){
	list* list = malloc(sizeof(struct list));
	list->head = NULL;
	list->tail = NULL;
	list->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	list->cv = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

	return list;
}

void destroyList(struct list* list){
	
	while(list->head != NULL){
		free(pullPacket(list));
	}
	
	pthread_mutex_destroy(&(list->mutex));
	pthread_cond_destroy(&(list->cv));

	free(list);
}

//nodes
int sendaskforfile(int sockfd, char* filename){
	// ASK:FILENAME;

	char* header = "ASK:";

	int size = strlen(filename) + strlen(header);

	char packet[size + 1];
	memset(packet, 0, size);

	memcpy(packet, header, strlen(header));

	strcat(packet, filename);

	if(write(sockfd, packet, size + 1) == -1){
		// failure
		return -1;
	}

	return 0;
}

int readOutPacket(int sockfd, char* buf){
	int rv = 0;

	if((rv = read(sockfd, buf, MAXBUFSIZE)) == -1){
		return -1;
	}

	return rv;
}

int decodePacket(char* packet){
	// ASK:FILENAME;

	char* copy = strdup(packet);

	char* header = strtok(copy, ":");
	fprintf(stderr, "%s\n", header);

	if(!strcmp(header, "ASK")){
		fprintf(stderr, "Received %s header\n", header);
		// send to every connection
	}
	else if(!strcmp(header, "SEARCH")){
		fprintf(stderr, "Received %s header\n", header);
	}
	else if(!strcmp(header, "RESP")){
		fprintf(stderr, "Received %s header\n", header);
		// compile list
	}
	else if(!strcmp(header, "DL")){
		fprintf(stderr, "Received %s header\n", header);
	}
	else if(!strcmp(header, "DLSEG")){
		fprintf(stderr, "Received %s header\n", header);
	}

//	free(copy);

	return 0;
}




// tracker & nodes
int recvpacket(int sockfd/*, list*/){
	// ASK:FILENAME;
	(void)sockfd;

	char buf[MAXBUFSIZE];

	if(read(sockfd, buf, MAXBUFSIZE) == -1){ // blocks here
		// failure
		return -1;
	}

	char* header = strtok(buf, ":");
	fprintf(stderr, "%s\n", header);

	if(!strcmp(header, "ASK")){
		fprintf(stderr, "Recvieved %s header\n", header);
//		sendsearchforfile(buf/*, list*/);
		recvpacket(sockfd);
	}
	else if(!strcmp(header, "SEARCH")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}
	else if(!strcmp(header, "RESP")){
		fprintf(stderr, "Recvieved %s header\n", header);
		// compile list
	}
	else if(!strcmp(header, "DL")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}
	else if(!strcmp(header, "DLSEG")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}

	return 0;
}

int sendsearchforfile(char* filename/*, list*/){
	// ASK:FILENAME;

	char* header = "SEARCH:";

	int size = strlen(filename) + strlen(header);

	char packet[size + 1];
	memset(packet, 0, size);

	memcpy(packet, header, strlen(header));

	strcat(packet, filename);

//	if(write(sockfd, packet, size + 1) == -1){
//		// failure
//		return -1;
//	}

	return 0;
}

size_t getfilesize(char* filename){

	size_t size = 0;

	if(access(filename, R_OK) == 0){
		// have file
		struct stat st;
		stat(filename, &st);
		size = st.st_size;

	}
	
	return size;

}

int sendhavefile(int sockfd, char* filename){
	// 
	(void)sockfd;

	char* header = "RESP:";


	// 16 means num of digits max filesize = 10^16 bytes
	char num[16];

	// int -> string representation
	sprintf(num, "%zu", getfilesize(filename));
	
	int size = strlen(num) + strlen(header);

	char packet[size + 1];

	memset(packet, 0, size);

	memcpy(packet, header, strlen(header));

	strcat(packet, num);

	fprintf(stderr, "%s\n", packet);
	

//	if(write(sockfd, packet, size + 1) == -1){
//		// failure
//		return -1;
//	}

	return 0;
}

/*
int sendnodeswithfile(int sockfd, char* filename){
	//
	(void)sockfd;

	char* header = "RESP:";


	// 16 means num of digits max filesize = 10^16 bytes
	char num[16];

	// int -> string representation
	sprintf(num, "%zu", getfilesize(filename));
	
	int size = strlen(num) + strlen(header);

	char packet[size + 1];

	memset(packet, 0, size);

	memcpy(packet, header, strlen(header));

	strcat(packet, num);

	fprintf(stderr, "%s\n", packet);
	

//	if(write(sockfd, packet, size + 1) == -1){
//		// failure
//		return -1;
//	}

	return 0;
}
*/
