/*
 *	tracker_server.c -- a stream socket server demo
 */

#include "utils.h"
#include "t_server.h"

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

list* head = NULL;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void sigchld_handler(int s){
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	
	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;

}

// get sockaddr, IPv4 or IPv6:

int t_server(int argc, char** argv){
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


		printf("socket in t_server:%d\n", new_fd);

		pthread_t new_thread;


		serve_struct *ss = malloc(sizeof(serve_struct));
		ss -> sockfd = new_fd;
		ss -> req_ip = strdup(s);

		pthread_create(&new_thread, NULL, t_serve, (void*)ss);

	}

}

void* t_serve(void* p){
	serve_struct *ss = p;
	int sockfd = (ss) -> sockfd;
	char *ip = (ss) -> req_ip;

	pthread_mutex_lock(&m);

	printf("Hold up im reading...\n");

	head = readPacket(sockfd, head, ip, NULL, NULL);

	list* curr = head;

	while(curr != NULL){
		printf("ip:%s, port:%s\n", curr->ip, curr->port);
		curr = curr->next;
	}

	pthread_mutex_unlock(&m);

	close(sockfd);
	free(p);

	return NULL;
}

