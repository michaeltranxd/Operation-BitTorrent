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
	char* ip;
	char* port;
	struct list* next;
}list;

list* connectAll(list* head, char* filename, int* numConnections);
int connectAndSend(list* node, char* filename);
list* newConnection(list* head, char* ip, char* port);
void addConnection(list* head, list* new_list);
list* removeConnection(list* head, list* remove);
list* findConnection(list* head, char* ip);
void destroyList(list* head);

int sendHelper(int sockfd, char* packet);
int readOutPacket(int sockfd, char* buf);
list* readPacket(int sockfd, list* head);

int parse_packet_header(char* buf);
list* decodePacket(char* buf, list* head);
void makePacket(char* buf, char* filename, char* ip, char* port, int packet_num);
int sendPacket(int sockfd, char* buf, char* filename, char* ip, char* port, int packet_num);
list* decodePacketNum(char* buf, int packet_num, list* head);

// types of packet
int ask_reqPacket(char* buf, char* filename, char* ip, char* port);
int resp_reqPacket(char* buf, char* filename);
int ask_availPacket(char* buf, char* filename);
int resp_availPacket(char* buf, char* filename);
int ask_dlPacket(char* buf, char* filename);
int resp_dlPacket(char* buf, char* filename);

#endif
