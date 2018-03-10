/*
 *	client_annotated.c -- a stream socket client demo
 *	The sample client_server code has been provided by Beeji and Michael,
 *	So i wont bother typing it out again.
 *	I will annotate the sample code a bit so that it makes more sense to me (and to show i've done the reading ./shrug). 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

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

int main(int argc, char* argv[]){
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if(argc != 2){
		fprintf(stderr, "usage: client hostname\n");
		exit(1);
	}


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
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

/* int socket(int domain, int type, int protocol)
 * domain: 	IPv4/ IPv6;	
 * type: 	stream or datagram;	
 * protocol: 	TCP or UDP
 * It returns a socket descriptor for later use
 *
 * (bind() is called on the server side)
 * (dont need bind() if you just need to connect)
 * int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
 * sockfd:	socket descriptor returned by socket()
 * my_addr:	contains "my" port and IP address
 * addrlen:	length in bytes of my_addr
 * It associates the socket with a specific port in "my" local machine. 
 *
 * int connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
 * sockfd: 	socket descriptor returned by socket()
 * serv_addr: 	contains destination port and IP address
 * addrlen:	length in bytes of serv_addr
 *
 */
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("client: socket");
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if(p == NULL){
		fprintf(stderr, "client: failted to connect\n");
		return 2;
	}

/* "Network to presentation/printable"
 * Convert the IP address into human-readable representation, like "192.0.2.33". 
 */
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);						// all done with this structure


/* (send() is called on the server side)
 * int send(int sockfd, const void *msg, int len, int flags)
 * sockfd:	the socket descriptor 
 * msg:		data intended to be sent
 * len:		length in bytes of the msg
 * flags:	check man page
 * On success, return the ACTUAL number of bytes successfully sent (might be less than len)
 * On error, return -1
 * The datagram version of this is sendto()
 * 
 *
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
 */
	if((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1){
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n", buf);

	close(sockfd);

	return 0;

}




