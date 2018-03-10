
#include "server.h"
#include "client.h"
#include "file_transfer.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *	The struct's sole purpose is to
 *	be able to give the server_thread
 *	function argc and argv
 *
 */
typedef struct argc_argv{
	int argc;
	char** argv;
}argcv;

typedef struct client_args{
	char* hostname;
	char* port;
}c_args;

void* server_thread_method(void* ptr){
	argcv* args = ptr;
	server(args->argc, args->argv);
	return ptr;
}

void* multi_client(void* ptr){
	c_args* args = ptr;
	client(args->hostname, args->port);
	free(args);
	return NULL;
}

void* client_thread_method(void* ptr){

	// we gonna be connecting if you want
	char buffer[100];
	char* hostname;
	char* port;
	size_t len;

	while(1){
		printf("hostname port:");
		fflush(stdin);

		if(fgets(buffer, 100, stdin) != NULL){

			hostname = strtok(buffer, " ");

			port = strtok(NULL, " ");

			if(port != NULL){ // only works if port is not null
				len = strlen(port);

				if(port != NULL && port[len - 1] == '\n')
					port[len - 1] = '\0';
			}

			c_args* args = malloc(sizeof(c_args));
			args->hostname = hostname;
			args->port = port;

			pthread_t new_thread;

			// we do not have to worry about error checking
			// because the client method does it for us! :^)
			pthread_create(&new_thread, NULL, multi_client, (void*)args);
			pthread_detach(new_thread);

		}
	}

	return ptr;
}

int main(int argc, char** argv){

	pthread_t server_thread;
	pthread_t client_thread;

	if(argc != 2){
		fprintf(stderr, "usage: server port\n");
		exit(1);
	}

	argcv* args = malloc(sizeof(argcv));
	args->argc = argc;
	args->argv = argv;

	pthread_create(&server_thread, NULL, server_thread_method, (void*)args);

	// After this we got a thread focused on server

	pthread_create(&client_thread, NULL, client_thread_method, (void*)NULL);

	// Below this line is cleanup (we can always pthread_exit(NULL))

	pthread_join(server_thread, NULL);
	pthread_join(client_thread, NULL);

	free(args);

	return 0;
}
