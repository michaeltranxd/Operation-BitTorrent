/**
 * 	server.c
 */

#ifndef TRACKER_SERVER_H
#define TRACKER_SERVER_H

#include "packet.h"

/**
 *	Method to handle child signals
 *
 */
void sigchld_handler(int s);

struct serve_struct{
	int sockfd;
}serve_struct;

void* serve(void* p);

/**
 *	Main method to setup server
 */
int server(int argc, char** argv);

#endif
