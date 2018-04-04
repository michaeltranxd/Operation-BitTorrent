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

typedef struct packet_executer_struct{
	list* listOfPackets;
	int* running;

}pe_struct;

void* packet_receiver(void* p);
void* packet_executer(void* p);

/**
 *	Main method to setup server
 */
int server(int argc, char** argv);


#endif
