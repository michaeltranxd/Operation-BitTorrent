
#include "p_server.h"
#include "client.h"

#include "peer.h"

#include "packet.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <ifaddrs.h>

#include <netinet/in.h>
#include <arpa/inet.h>

void* server_thread_method(void* ptr){
	argcv* args = ptr;
	p_server(args->argc, args->argv);
	return ptr;
}

void* multi_client(void* ptr){
	c_args* args = ptr;
	char buf[1024];
	getConnection(args->hostname, args->port);
	client(args->hostname, args->port,  args->myport, args->filename, buf, 0, 0, ASK_REQ);
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
	memset(buffer, 0, 100);

	mca* mca = ptr;

	sleep(1);

	while(1){
		printf("hostname port filename:");
		fflush(stdin);

		if(fgets(buffer, 100, stdin) != NULL){

			hostname = strtok(buffer, " ");

			port = strtok(NULL, " ");

			filename = strtok(NULL, " ");

			if(filename != NULL){ // only works if port is not null
				len = strlen(filename);

				if(filename != NULL && filename[len - 1] == '\n')
					filename[len - 1] = '\0';
			}

			printf("hostname:%s port:%s filename:%s\n", hostname, port, filename);

			c_args* args = malloc(sizeof(c_args));
			args->hostname = strdup(hostname);
			args->port = strdup(port);
			args->filename = strdup(filename);
			args->myip = strdup(mca->myip);
			args->myport = strdup(mca->myport);
			
			pthread_t new_thread;

			// we do not have to worry about error checking
			// because the client method does it for us! :^)
			pthread_create(&new_thread, NULL, multi_client, (void*)args);
			pthread_detach(new_thread);

		}
		memset(buffer, 0, 100);
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

	tasks_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	add_task_cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

	struct ifaddrs *addrs, *tmp;


	mca* mca = malloc(sizeof(mca));
	mca->myport = argv[1];

	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
			printf("%s: %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
			mca->myip = inet_ntoa(pAddr->sin_addr);
			//break;
		}

		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);



	int i = 0;
	for (; i < MAXTASKSCOUNT; i++){
		task_conds[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
	}

	pthread_create(&server_thread, NULL, server_thread_method, (void*)args);

	// After this we got a thread focused on server

	pthread_create(&client_thread, NULL, client_thread_method, (void*)mca);

	// Below this line is cleanup (we can always pthread_exit(NULL))

	pthread_join(server_thread, NULL);
	pthread_join(client_thread, NULL);

	free(args);

	return 0;
}
