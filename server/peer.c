
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
			if(hostname == NULL)
				continue;

			port = strtok(NULL, " ");
			if(port == NULL)
				continue;

			filename = strtok(NULL, " ");
			if(filename == NULL)
				continue;

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

	if (pthread_mutex_init(&task_lock, NULL) != 0) {
		perror("Failed mutex_init() for task_lock");
	}
	if (pthread_cond_init(&add_task_cond, NULL) != 0) {
		perror("Failed cond_init() for add_task_cond");
	}

	if (pthread_cond_init(&test_cond, NULL) != 0)
		perror("Failed cond_init() for test_cond");
	tasks_name = malloc(sizeof(char *) * MAXTASKSCOUNT);
	tasks_count = malloc(sizeof(int) * MAXTASKSCOUNT);
	
	task_conds = malloc(sizeof(pthread_cond_t) * MAXTASKSCOUNT);
	int i = 0;
	for (; i < MAXTASKSCOUNT; i ++) {
		tasks_name[i] = NULL;
		tasks_count[i] = 0;
		if (pthread_cond_init(&task_conds[i], NULL) != 0)
			perror("Failed cond_init() for task_cond[]");
		printf("tasks_name[%d] is %s\n", i, tasks_name[i]);
	}
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




	pthread_create(&server_thread, NULL, server_thread_method, (void*)args);

	// After this we got a thread focused on server

	pthread_create(&client_thread, NULL, client_thread_method, (void*)mca);

	// Below this line is cleanup (we can always pthread_exit(NULL))

	pthread_join(server_thread, NULL);
	pthread_join(client_thread, NULL);

	free(args);
	printf("Test cond_wait() on test_cond at the end of peer.c\n");
	pthread_mutex_lock(&task_lock);
	printf("Finished mutex_lock()\n");
	pthread_cond_wait(&test_cond, &task_lock);
	printf("Finished cond_wait()\n");
	pthread_mutex_unlock(&task_lock);
	printf("Finished mutex_unlock()\n");
	pthread_mutex_destroy(&task_lock);
	pthread_cond_destroy(&add_task_cond);
	for (i = 0; i < MAXTASKSCOUNT; i ++) {
		pthread_cond_destroy(&task_conds[i]);
	}

	return 0;
}
