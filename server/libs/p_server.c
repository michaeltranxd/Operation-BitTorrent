
/*
 *	p_server.c -- a stream socket server demo
 */

#include "utils.h"
#include "p_server.h"

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

#define MAX_CLIENTS 10

#define BACKLOG 10

#define MAXBUFSIZE 1024

void sigchld_handler(int s){
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	
	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;

}

// get sockaddr, IPv4 or IPv6:

int p_server(int argc, char** argv){
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


		pthread_t new_thread;

		int* fd = malloc(sizeof(int));
		*fd = new_fd;

		pthread_create(&new_thread, NULL, p_serve, (void*)fd);

	}

}

void* p_serve(void* p){
	int sockfd = *(int*)p;
	printf("start p_server, before readPacket()\n");
	printf("addr of tasks_name: %p; tasks_count: %p; task_lock: %p\n", &(tasks_name[0]), &(tasks_count[0]), &task_lock);
	printf("call print_tasks_info() function\n");
	print_tasks_info();
	printf("manually call print_tasks_info()\n");
	//printf("task name at tasks_name[%d] is: %s, task count at tasks_count[%d] is %d\n", 1, tasks_name[1], 1, tasks_count[0]);
	int test_itr = 0;
	for (; test_itr < MAXTASKSCOUNT; test_itr ++) {
		printf("task name at tasks_name[%d] is: %s, task count at tasks_count[%d] is %d\n", test_itr, tasks_name[test_itr], test_itr, tasks_count[test_itr]);
	}
	

	readPacket(sockfd, NULL, NULL);

	close(sockfd);
	free(p);

	return NULL;
}

