/*
 *	packet.h
 */

#ifndef PACKET_H
#define PACKET_H


int recvpacket(int sockfd);
int sendaskforfile(int sockfd, char* filename);
int sendsearchforfile(int sockfd, char* filename/*, list*/);
size_t getfilesize(char* filename);
int sendhavefile(int sockfd, char* filename);

#endif
