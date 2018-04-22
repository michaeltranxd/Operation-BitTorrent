/*
 *	packet.h
 */

#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>


#include "client.h"
#include "file_transfer.h"

#define MAXBUFSIZE 1024

#define ASK_REQ 		1
#define RESP_REQ 		2
#define ASK_AVAIL 		3
#define RESP_AVAIL 		4
#define ASK_DL			5
#define START_SD		6


#define MAXTASKSCOUNT 10

pthread_mutex_t tasks_lock;
pthread_cond_t task_conds[MAXTASKSCOUNT];
pthread_cond_t add_task_cond;
char **tasks_name;
int *tasks_count;

typedef struct list{
	char *ip;
	char *port;
	struct list* next;
}list;

list* connectAll(list* head, char* filename, int* numConnections, char* buf, char* ip);
long long connectAndSend(list* node, char* filename);
list* newConnection(list* head, char* ip, char* port);

void addConnection(list* head, list* new_list);
list* removeConnection(list* head, list* remove);
list* findConnection(list* head, char *ip);
void destroyList(list* head);

int sendHelper(int sockfd, char* packet);
int readOutPacket(int sockfd, char* buf);
list* readPacket(int sockfd, list* head, char *req_ip, int* tasks_count, char** tasks_name);

int parse_packet_header(char* buf);
list* decodePacket(int dl_sockfd, char* buf, list* head, char *req_ip, int* tasks_count, char**tasks_name);
void makePacket(char *buf, char *filename, char *ip, char *port, size_t filesize, int index, int packet_num);
long long sendPacket(int sockfd, char* buf, char* filename, char* ip, char* port, size_t filesize, int index, int packet_num);

// changed the way of reading packet (it is read byte by byte now)


// need to pass in the current sockfd that receives START_SD, so i pass a sockfd into related functions.
// This is due to how receiving START_SD work. After receiving START_SD packet the server will call recv_file() on the same sockfd, so the server needs to know the sockfd.
list* decodePacketNum(int dl_sockfd, char *buf, int packet_num, list* head, char *req_ip, int* tasks_count, char** tasks_name);

// types of packet
int ask_reqPacket(char *buf, char *filename, char *port);

// added a filesize para for this packet
int resp_reqPacket(char *buf, char *filename, char* req_ip, char* req_port);

// ignore changes in these two packet
int ask_availPacket(char *buf, char *filename);
int resp_availPacket(char *buf, char *filename, size_t filesize);

// major update in these two packets. Delete the "resp_dlPacket", dont need it.
int ask_dlPacket(char *buf, char *filename, size_t filesize, size_t index, char* ip, char* port);
int start_sdPacket(char *buf, char *filename, size_t filesize, size_t index);

#endif
