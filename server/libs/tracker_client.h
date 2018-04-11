/*
 *	client.h 
 */

#ifndef TRACKER_CLIENT_H
#define TRACKER_CLIENT_H

/**
 *	Main method for client that runs the client
 *	portion
 */
int t_client(char* hostname, char* port, char* filename, char* buf, int packet_num);

#endif
