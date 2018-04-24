
#include "t_server.h"
#include "packet.h"
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

void* server_thread_method(void* ptr){
	argcv* args = ptr;
	t_server(args->argc, args->argv);
	return ptr;
}

int main(int argc, char** argv){

	pthread_t server_thread;

	if(argc != 2){
		fprintf(stderr, "usage: server port\n");
		exit(1);
	}

	argcv* args = malloc(sizeof(argcv));
	args->argc = argc;
	args->argv = argv;

	pthread_create(&server_thread, NULL, server_thread_method, (void*)args);

	// After this we got a thread focused on server

	// Below this line is cleanup (we can always pthread_exit(NULL))

	pthread_join(server_thread, NULL);

	free(args);

	return 0;
}
