/*
 *	client.c -- a stream socket client demo
 */

#include "packet.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>


//#define PORT "3490" // the port client will be connecting to

#define FILENAME "testget.txt" // hard coded filename to receive

int getConnection(char* hostname, char* port){
	int sockfd, numbytes;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if(hostname == NULL || port == NULL){
		fprintf(stderr, "usage: hostname port\n");
		return -1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("client: socket");
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if(p == NULL){
		fprintf(stderr, "client: failed to connect\n");
		return -1;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);						// all done with this structure

	(void)numbytes;


	printf("tracker success! Connected!\n");

	return sockfd;

}

int t_client(char* hostname, char* port, char* filename, char* buf, int packet_num){
	int sockfd, rv;

	if((sockfd = getConnection(hostname, port)) == -1){ // failed
		return -1;
	}

	rv = sendPacket(sockfd, buf, filename, hostname, port, packet_num);	
	
	close(sockfd);

	return rv;

}


