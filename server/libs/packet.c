
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "packet.h"

#define MAXBUFSIZE 1024

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

// tracker & nodes
int recvpacket(int sockfd){
	// ASK:FILENAME;
	(void)sockfd;

	char buf[MAXBUFSIZE];

	if(read(sockfd, buf, MAXBUFSIZE) == -1){
		// failure
		return -1;
	}

	char* header = strtok(buf, ":");
	fprintf(stderr, "%s\n", header);

	if(!strcmp(header, "ASK")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}
	else if(!strcmp(header, "SEARCH")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}
	else if(!strcmp(header, "RESP")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}
	else if(!strcmp(header, "DL")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}
	else if(!strcmp(header, "DLSEG")){
		fprintf(stderr, "Recvieved %s header\n", header);
	}

	return 0;
}

int sendsearchforfile(int sockfd, char* filename/*, list*/){
	// ASK:FILENAME;
	(void)sockfd;

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
