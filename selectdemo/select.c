/*
 *	select.c -- a select() demo
 *
 *	Waits for user to press enter
 *	else it times out after 2.5 seconds
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN 0			// file descriptor for standard input

int main(void){
	
	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 2;
	tv.tv_usec = 500000;

	FD_ZERO(&readfds);
	FD_SET(STDIN, &readfds);

	// dont care about writefds and exceptfds:
	select(STDIN + 1, &readfds, NULL, NULL, &tv);

	if(FD_ISSET(STDIN, &readfds))
		printf("a key was pressed!\n");
	else
		printf("Timed out.\n");

	return 0;
}
