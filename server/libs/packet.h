/*
 *	packet.h
 */

#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>
#include <pthread.h>

#define ASK_REQ 		1
#define RESP_REQ 		2
#define ASK_AVAIL 		3
#define RESP_AVAIL 		4
#define ASK_DL			5
#define RESP_DL			6

typedef struct list{
	char *ip;
	char *port;
	struct list* next;
}list;

list* connectAll(list* head, char *filename, int* numConnections);
int connectAndSend(list* node, char *filename);
list* newConnection(list* head, char *ip, char *port);
void addConnection(list* head, list* new_list);
list* removeConnection(list* head, list* remove);
list* findConnection(list* head, char *ip);
void destroyList(list* head);

// changed the way of reading packet (it is read byte by byte now)
int serverHelper(int sockfd, char *hostname); 
int clientHelper(int sockfd, char *filename, char *myip, char *myserverport);
int sendHelper(int sockfd, char *packet);
int readOutPacket(int sockfd, char *buf);
list* readPacket(int sockfd, list* head);

int parse_packet_header(char *buf);
list* decodePacket(char *buf, list* head);
void makePacket(char *buf, char *filename, char *ip, char *port, size_t filesize, int index, int packet_num);
int sendPacket(int sockfd, char *buf, char *filename, char *ip, char *port, size_t filesize, int index, int packet_num);

// need to pass in the current sockfd that receives START_SD, so i pass a sockfd into related functions.
// This is due to how receiving START_SD work. After receiving START_SD packet the server will call recv_file() on the same sockfd, so the server needs to know the sockfd.
list* decodePacketNum(int sockfd, char *buf, int packet_num, list* head);

// types of packet
int ask_reqPacket(char *buf, char *filename, char *ip, char *port);

// added a filesize para for this packet
int resp_reqPacket(char *buf, char *filename, size_t filesize);

// ignore changes in these two packet
int ask_availPacket(char *buf, char *filename);
int resp_availPacket(char *buf, char *filename);

// major update in these two packets. Delete the "resp_dlPacket", dont need it.
int ask_dlPacket(char *buf, char *filename, size_t filesize, size_t index);
int start_sdPacket(char *buf, char *filename, size_t filesize, size_t index);

#endif