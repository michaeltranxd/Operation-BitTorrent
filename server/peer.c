
#include "p_server.h"
#include "client.h"

#include "peer.h"

#include "packet.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void* server_thread_method(void* ptr){
	argcv* args = ptr;
	p_server(args->argc, args->argv);
	return ptr;
}

void* multi_client(void* ptr){
	c_args* args = ptr;
	char buf[1024];
	client(args->hostname, args->port, args->filename, buf, 0, 0, ASK_REQ);
	free(args);
	return NULL;
}

void* client_thread_method(void* ptr){

	// we gonna be connecting if you want
	char buffer[100];
	char* hostname;
	char* port;
	char* filename;
	size_t len;

	sleep(1);

	while(1){
		printf("hostname port filename:");
		fflush(stdin);

		if(fgets(buffer, 100, stdin) != NULL){

			hostname = strtok(buffer, " ");

			port = strtok(NULL, " ");

			filename = buffer;

			if(port != NULL){ // only works if port is not null
				len = strlen(port);

				if(port != NULL && port[len - 1] == '\n')
					port[len - 1] = '\0';
			}

			c_args* args = malloc(sizeof(c_args));
			args->hostname = hostname;
			args->port = port;
			args->filename = filename;

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
