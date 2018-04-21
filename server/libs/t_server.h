/**
 * 	server.c
 */

#ifndef T_SERVER_H
#define T_SERVER_H

#include "packet.h"

/**
 *	Method to handle child signals
 *
 */
void sigchld_handler(int s);

typedef struct serve_struct{
	int sockfd;
	char *req_ip;
}serve_struct;

void* t_serve(void* p);

/**
 *	Main method to setup server
 */
int t_server(int argc, char** argv);

#endif
