/**
 * 	server.c
 */

#ifndef SERVER_H
#define SERVER_H

/**
 *	Method to handle child signals
 *
 */
void sigchld_handler(int s);

/**
 *	Main method to setup server
 */
int server(int argc, char** argv);

#endif
