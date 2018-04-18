
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "packet.h"

void testPacketList(){
	struct list* list = createList();

	char* m = calloc(sizeof(char), 2);
	char* o = calloc(sizeof(char), 2);
	char* p = calloc(sizeof(char), 2);

	m[0] = 'M';
	o[0] = 'O';
	p[0] = 'P';

	struct packets* p1 = createPacket(m);
	struct packets* p2 = createPacket(o);
	struct packets* p3 = createPacket(p);

	fprintf(stderr, "Packet 1 holds: %s\n", p1->packet_string);
	fprintf(stderr, "Packet 2 holds: %s\n", p2->packet_string);
	fprintf(stderr, "Packet 3 holds: %s\n\n", p3->packet_string);

	pushPacket(list, p1);
	pushPacket(list, p2);
	pushPacket(list, p3);

	p3 = pullPacket(list);
	p2 = pullPacket(list);
	p1 = pullPacket(list);

	fprintf(stderr, "Packet 1 holds: %s\n", p1->packet_string);
	fprintf(stderr, "Packet 2 holds: %s\n", p2->packet_string);
	fprintf(stderr, "Packet 3 holds: %s\n", p3->packet_string);

	destroyPacket(p1);
	destroyPacket(p2);
	destroyPacket(p3);

	destroyList(list);

}

int main(){
	
	//testPacketList();
	
	decodePacket("ASK:localhost");

	return 0;
}
