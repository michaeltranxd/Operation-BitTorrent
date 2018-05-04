#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void* get_in_addr(struct sockaddr* sa);

#endif
