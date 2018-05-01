/*
 *	client.h 
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>

int getConnection(char* hostname, char* port);
/**
 *	Main method for client that runs the client
 *	portion
 */
long long client(char* hostname, char* port, char *req_port, char* filename, char* buf, size_t filesize, int index, int packet_num);

#endif
