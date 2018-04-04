/*
 *	tracker_server.c -- a stream socket server demo
 */

#include "server.h"
#include "utils.h"
#include "tracker_server.h"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

//define PORT "3490"							// the port users will be connecting to

#define FILENAME "testsend.txt"

#define MAX_CLIENTS 10

#define BACKLOG 10

#define MAXBUFSIZE 1024

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* packet_executer(void* p){ // executes packet

	pe_struct* pe_struct = p;
	list* listOfPackets = pe_struct->listOfPackets;
	int* running = pe_struct->running;

	packets* packet = NULL;

	int connected = 1;

	while(connected){

		packet = pullPacket(listOfPackets);

		if(packet == NULL){
			pthread_cond_wait(&(listOfPackets->cv), &(listOfPackets->mutex));
			if(*running == 0)
				connected = 0;
			pthread_mutex_unlock(&(listOfPackets->mutex));
		}
		else{
			// within the methods we should do whats necessary maybe?
			decodePacket(packet->packet_string);
		}

	}

	return NULL;

}

void* packet_receiver(void* p){ // receives packet on socket p
	int* clientId = (int*)p;

	// socket would be clients[clientId]
	

	int returnVal = 1;

	list* listOfPackets = createList();

	// struct for packet_executer to have running
	pe_struct* pe_struct = malloc(sizeof(struct packet_executer_struct));
	pe_struct->listOfPackets = listOfPackets;
	pe_struct->running = malloc(sizeof(int));
	*pe_struct->running = 1;



	pthread_t newthread;
	pthread_create(&newthread, NULL, packet_executer, (void*)pe_struct);

	char buf[MAXBUFSIZE];

	while(returnVal > 0){


		returnVal = readOutPacket(clients[*clientId], buf);

		if(returnVal > 0){

			char* packet_string = calloc(sizeof(char), returnVal + 1);

			memcpy(packet_string, buf, returnVal);

			pushPacket(listOfPackets, createPacket(packet_string));
		
			fprintf(stderr, "returnVal was:%d, packet_string:%s\n", returnVal, packet_string);
		
		}
	}

	pthread_mutex_lock(&mutex);
	clients[*clientId] = -1;
	pthread_mutex_unlock(&mutex);

	pthread_join(newthread, NULL);

	destroyList(listOfPackets);

	free(clientId);
	free(pe_struct->running);
	free(pe_struct);

	return NULL;
}



void sigchld_handler(int s){
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	
	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;

}

// get sockaddr, IPv4 or IPv6:

int server(int argc, char** argv){
	int sockfd, new_fd;						// listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;		// connectors' address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	if(argc != 2){
		fprintf(stderr, "usage: server port\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints); 		// Make sure struct has nothing inside of it
	hints.ai_family = AF_UNSPEC;			// Dont care which version for ip
	hints.ai_socktype = SOCK_STREAM;		// TCP connection
	hints.ai_flags = AI_PASSIVE;			// use my ip

	if((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
					perror("server: socket");
					continue;
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); 				// all done with this structure

	if(p == NULL){
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if(listen(sockfd, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;		// reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}

	for(int i = 0; i < MAX_CLIENTS; i++){
		clients[i] = -1;
	}

	printf("server: waiting for connections...\n");

	while(1){
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
		if(new_fd == -1){
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);


		int isAdded = 0;
		for(int i = 0; i < MAX_CLIENTS; i++){
			pthread_mutex_lock(&mutex);
			if(clients[i] == -1){
				clients[i] = new_fd;
				isAdded = 1;

				int* temp = malloc(sizeof(int));
				*temp = i;

				pthread_t newthread;
				pthread_create(&newthread, NULL, packet_receiver, (void*)temp);
				pthread_detach(newthread);
				pthread_mutex_unlock(&mutex);
				break;
			}
			pthread_mutex_unlock(&mutex);
		}
		if(isAdded == 0){ // did not have open spot
			close(new_fd); // closes connection
			fprintf(stderr, "sorry full!\n");
		}
	}
	

}

