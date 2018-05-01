/**
 * 	server.c
 */

#ifndef P_SERVER_H
#define P_SERVER_H


/**
 *	Method to handle child signals
 *
 */
void sigchld_handler(int s);

struct serve_struct{
	int sockfd;
}serve_struct;

void* p_serve(void* p);

/**
 *	Main method to setup server
 */
int p_server(int argc, char** argv);

#endif
