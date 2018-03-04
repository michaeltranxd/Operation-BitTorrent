/*
 * 	This is a reference page to
 * 	all structs and contains certain
 * 	information to things
 *	
 *	PLEASE do not try to compile this
 *	because it will not compile lol.
 *
 *	-Michael Tran
 *
 */

// select() can return "ready-to-read" then not actually be ready to read. Will block on read() then.
// workaround: set O_NONBLOCK flag on receiving socket so it errors with EQOULDBLOCK
// see fcntl() ref page


/*	#includes
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h> // unblock socket (aka make it not sleep, sleep is like how you wait for packet and sit there waiting)



/*	 GLOBAL
 *	VARIABLES
 */

AF_INET									// address family IPv4		Usage: struct sockaddr_in
AF_INET6								// address family IPv6
AF_UNSPEC								// dont care IPv4 or IPv6
PF_INET									// protocol family IPv4		Usage: call to socket()
PF_INET6								// protocol family IPv6
INET_ADDRSTRLEN							// space to hold the IPv4 string
INET6_ADDRSTRLEN						// space to hold the IPv6 string
INADDR_ANY								// IPv4 local host address
IN6ADDR_ANY_INIT						// initialize struct in6_addr
SOCK_STREAM								// TCP Reliable two-way connection-based byte streams
SOCK_DGRAM								// UDP Connectionless, unreliable messages of fixed max length
AI_PASSIVE								// Fill in the IP for me
SOL_SOCKET								// Item searched for in the socket itself
SO_REUSEADDR							// Socket option to reuse address


/* 	Methods
 *    and
 * Functions
 */

int select(int numfd,					// value of highest file descriptor plus one
		   fd_set* readfds,				// file descriptors watched if ready to read from
		   rd_set* writefds,			// file descriptors watched if space ready to write to
		   fd_set *exceptfds,			// file descriptors watched for exceptional conditions
		   struct timeval* timeout)		// timeout, if exceeded it'll return

FD_SET(int fd, fd_set* set)				// Add fd to set
FD_CLR(int fd, fd_set* set)				// Remove fd from the set
FD_ISSET(int fd, fd_set* set)			// Return true if fd is in the set
FD_ZERO(fd_set* set)					// Clear all entries from the set


fcntl(int sockfd, int cmd, ...)			// unblocks a socket file descriptor

uint16_t htons(uint16_t hostshort) 		// converts hostshort from host byte order to network byte order
uint32_t htonl(uint32_t hostlong) 		// converts hostlong from host byte order to network byte order
uint16_t ntohs(uint16_t netshort) 		// converts netshort from network byte order to host byte order
uint32_t ntohl(uint32_t netlong) 		// converts netlong from network byte order to host byte order

in_addr_t inet_addr(const char*)

char* inet_ntoa(struct in_addr)

// "presentation to network"
int inet_pton(int af,					// ai_family
			  const void* src,			// IP address string
			  void* dst)				// &sin_addr

// "network to presentation"
const char* inet_ntop(
		int af,							// ai_family
		const void* restrict src,		// &sin_addr
		char* restrict dst,				// space to hold IPv4/IPv6 string
		socklen_t size)					// size of IPv4/IPv6 string (use final variables above)


// returns 0 on success, non-zero on error
int getaddrinfo(const char* node,		// e.g. "www.example.com" or IP
				const char* service,	// e.g. "http" or port number
				const struct addrinfo* hints,
				struct addrinfo** res)

void freeaddrinfo(struct addrinfo* res) // frees memory allocated for dynamically allocated linked list res

// returns a file descriptor for a socket, -1 on error
int socket(int domain, 					// IPv4/IPv6
		   int type, 					// stream/datagram
		   int protocol) 				// TCP/UDP

// When using bind, ports can be 1025-65535 (if not already used)
// returns 0 on success, -1 on error
int bind(int sockfd,					// socket file descriptor returned by socket()				
	   	 struct sockaddr* my_addr,		// struct sockaddr containing info about address, namely, port, IP address
		 int addrlen)					// length of bytes of that address

// returns 0 on success, -1 on error
int connect(int sockfd,					// socket file descriptor returned by socket()
			struct sockaddr* serv_addr, // struct sockaddr containing destination port and IP address
			int addrlen)				// length in bytes of server address structure

// returns 0 on success, -1 on error
int listen(int sockfd,					// socket file descriptor returned by socket()
		   int backlog)					// number of connections allowed on incoming queue

// returns 0 on success, -1 on error
int accept(int sockfd,					// socket file descriptor that is the listen()ing socket descriptor
		   struct sockaddr* addr,		// struct sockaddr_storage, info about incoming connecting will go 
		   								// (can determine which host calls you and which port)
		   sockletn_t* addrlen)			// local integer variable, should be set to sizeof(struct sockaddr_storage)
										// before passed to accept()

// returns amount of bytes actually sent
int send(int sockfd,					// socket file descriptor you want to send data to
		 const void* msg,				// pointer to data you want to send
		 int len,						// length of data in bytes
		 int flags)						// just set flags to 0 (see send() man page for more info)

// returns amount of bytes actually received
int recv(int sockfd,					// socket file descriptor to read from
		 void* buf,						// buffer to read information into
		 int len,						// max length of the buffer
		 int flags)						// just set flags to 0 (see man page for more info about flags)

// returns amount of bytes actually sent (for DGRAM)
int sendto(int sockfd,					// socket file descriptor you want to send data to
		   const void* msg,				// pointer to data you want to send
		   int len,						// length of data in bytes
		   unsigned int flags,			// just set flags to 0 (see man page for info about flags)
		   const struct sockaddr* to,	// pointer to struct sockaddr that contains destination IP address and port
		   socklen_t tolen)				// int deep-down can be set to sizeof* to or sizeof(struct sockaddr_storage)

// returns number of bytes actually received (for DGRAM)	
int recv(int sockfd,					// socket file descriptor to read from
		 void* buf,						// buffer to read information into
		 int len,						// max length of the buffer
		 unsigned int flags,			// just set flags to 0 (see man page for more info about flags)
		 struct sockaddr* from,			// ptr to local struct sockaddr_storage, will be filled w/ IP address and port of originating machine
		 int* fromlen)					// pointer to local int, should be initialized to sizeof* from orr sizeof(struct sockaddr_storage)

// returns 0 on success, -1 on error
int close(int sockfd)						// socket file descriptor to close, prevents read and writes to socket

// returns 0 on success, -1 on error
int shutdown(int sockfd,				// socket file descriptor to shutdown
			 int how)					// 0 - Further receives are disallowed
										// 1 - Further sends are disallowed
										// 2 - Further sends and receives are disallowed (like close())

// returns 0 on success, -1 on error
int getpeername(int sockfd,				// socket file descriptor of connected stream socket
				struct sockaddr* addr	// ptr to struct sockaddr or struct sockaddr_in, holds info about other side of connection
				int* addrlen)			// pointer to an int, should be initialized to sizeof* addr or sizeof(struct sockaddr)

// returns 0 on success, -1 on error
int gethostname(char* hostname,			// ptr to array of chars containing hostname upon function's return
	   			size_t size)			// length in bytes of hostname array


/*
 *	Structs
 */

struct addrinfo{
	int 			 	ai_flags;		// AI_PASSIVE, AI_CANONNAME, etc.
	int 			 	ai_family;		// AF_INET, AF_INET6, AF_UNSPEC
	int 			 	ai_socktype;	// SOCK_STREAM, SOCK_DGRAM
	int 			 	ai_protocol;	// use 0 for "any"
	size_t 			 	ai_addrlen;		// size of ai_addr in bytes
	struct sock addr*   ai_addr;		// struct sockaddr_in or _in6
	char*				ai_canonname;   // full canonical hostname

	struct addrinfo*	ai_next;		// linked list, next node
};

struct sock addr{
	unsigned short	 	sa_family;		// address family, AF_xxx
	char			 	sa_data[14];	// 14 bytes of protocol address
};

// IPv4 only
struct sockaddr_in{
	short int 		 	sin_family;		// Address family, AF_INET
	unsigned short int	sin_port;		// Port number
	struct in_addr		sin_addr;		// Internet address
	unsigned char		sin_zero[8];	// Same size as struct sockaddr
};

// IPv6 only
struct sockaddr_in6{
	sa_family_t			sin6_family;	// address family, AF_INET6
	in_port_t			sin6_port;		// port number, Network Byte Order
	uint32_t			sin6_flowinfo;	// IPv6 flow information
	struct in6_addr		sin6_addr;		// IPv6 address
	uint32_t			sin6_scope_id;  // Scope ID
};

struct in6_addr{
	unsigned char 		s6_addr[16];	// IPv6 address
};

struct sockaddr_storage{
	sa_family_t 		ss_family;		// address family

	// all this is padding, implementation specific, ignore it:
	char				__ss_pad1[_SS_PAD1SIZE];
	int64_t				__ss_align;
	char				__ss_pad2[_SS_PAD2SIZE];
};

struct timeval{
	int tv_sec;							// seconds
	int tv_usec;						// microseconds
}

		





