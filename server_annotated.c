/*
 *	server_annotated.c -- a stream socket server demo
 *	The sample client_server code has been provided by Beeji and typed by Michael,
 *	So i wont bother typing it out again.
 *	I will annotate the sample code a bit so that it makes more sense to me (and to show i've done the reading lol). 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"							// the port users will be connecting to

#define BACKLOG 10 							// how many pending connections queue will hold

void sigchld_handler(int s){
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	
	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;

}

/* Both sockaddr and sockaddr_storage can be casted into/from sockaddr_in  (for IPv4)/ sockaddr_in6  (IPv6)
 * ???why not always use the bigger
 *
 * struct sockaddr {
 *	unsigned short	sa_family;		// address family (AF_INET or AF_INET6; could also be other things)
 *	char		sa_data[14];		// stores the protocol address 
 * }
 *
 * A parallel but bigger struct is sockaddr_storage. 
 * struct sockaddr_storage {
 *	sa_family_t ss_family; 			// address family (AF_INET or AF_INET6)
 *
 *	//some padding stuff that Beeji suggests to ignore
 *	char		__ss_pad1_[_SS_PAD1SIZE];
 *	int64_t		__ss_align;
 *	char		__ss_pad2_[_SS_PAD2SIZE];
 * }
 *
 * get sockaddr, IPV4 or IPV6
 */
void* get_in_addr(struct sockaddr* sa){
	if(sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){
	int sockfd, new_fd;						// listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;		// connectors' address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints); 		// Make sure struct has nothing inside of it
	hints.ai_family = AF_UNSPEC;			// Dont care which version for ip
	hints.ai_socktype = SOCK_STREAM;		// TCP connection
	hints.ai_flags = AI_PASSIVE;			// use my ip

/* ai stands for struct addrinfo
 * struct addrinfo {
 *	int 		ai_flags;
 *	int 		ai_family; 		// AF_INET, AF_INET6, AF_UNSPEC(if you dont know which specific family it is, as is the case in hints)
 *	int 		ai_socktype;		// SOCK_STREAM, SOCK_DGRAM
 *	int		ai_protocol;		// use 0 for "any"
 *	size_t		ai_addrlen;		// size of ai_addr in bytes
 *	struct sockaddr	*ai_addr;		// struct sockaddr_in or sockaddr_in6.
 *	char		*ai_canonname;		// ???what is this
 *
 *	struct addrinfo	*ai_next;		// pointer to next node
 * }
 *
 */
	if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

/* int socket(int domain, int type, int protocol)
 * domain: 	IPv4/ IPv6;	
 * type: 	stream or datagram;	
 * protocol: 	TCP or UDP
 * It returns a socket descriptor for later use
 *
 * int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
 * sockfd:	socket descriptor returned by socket()
 * my_addr:	contains "my" port and IP address
 * addrlen:	length in bytes of my_addr
 * It associates the socket with a specific port in "my" local machine. 
 * Prolly dont need bind() if you just need to connect (as is the case in the client example)
 *
 * To make the port reusable, use setsockopt()
 *
 * int connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
 * sockfd: 	socket descriptor returned by socket()
 * serv_addr: 	contains destination port and IP address
 * addrlen:	length in bytes of serv_addr
 *
 */
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
					perror("server: socket");
					continue;
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); 				// all done with this structure

	if(p == NULL){
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

/* int listen(int sockfd, int backlog)
 * sockfd:	the socket descriptor after being bound by bind()
 * backlog:	maximum number of connections allowed in que
 * Incoming connections will stay in que until being accepted by accept()
 *
 * int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
 * sockfd:	the socket descriptor after being bound by bind()
 * addr:	stores info of the incoming connection (which host, which port)
 * addrlen:	length in bytes of struct addr
 * On success, return a new socket descriptor to handle this specific connection (while the original socket descriptor is listening (via listen()) to more connection)
 * 
 * int send(int sockfd, const void *msg, int len, int flags)
 * sockfd:	the socket descriptor 
 * msg:		data intended to be sent
 * len:		length in bytes of the msg
 * flags:	check man page
 * On success, return the ACTUAL number of bytes successfully sent (might be less than len)
 * On error, return -1
 * The datagram version of this is sendto()
 *
 * (recv() is called in the client side)
 * int recv(int sockfd, void *buf, int len, int flags)
 * sockfd: 	the socket descriptor
 * buf:		the received info will be written into buf
 * len:		maximum length of buffer
 * flags:	check man page
 * On success, return the number of bytes successfully read into the buffer
 * On error, return -1
 * In case the host closed connection, returns 0
 * The datagram version of this is recvfrom()
 *
 * int shutdown (int sockfd, int how)
 * sockfd:	the socket descriptor the user wants to shutdown
 * how:		could be 0 (disallow further receives), 1 (disallow further sends), 2 (disallow sends and receives, like close())
 * Change the usability of sockfd. 
 * Still need to use close() it in the end
 * 
 */
	if(listen(sockfd, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;		// reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1){
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
		if(new_fd == -1){
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if(!fork()){						// child process
			close(sockfd); 					// child doesnt need listener
			if(send(new_fd, "Hello, world!", 13, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);						// parent doesnt need this
	}

}
