/**
 * 	server.c
 */

#ifndef TRACKER_SERVER_H
#define TRACKER_SERVER_H

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
