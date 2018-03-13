
#include "file_transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXDATASIZE 256	// max number of bytes we can get at once
// consider increasing the size limit, read https://en.wikipedia.org/wiki/Maximum_transmission_unit

int send_file(char* filename, int sockfd){
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		printf("Fail to open file %s\n", filename);
		exit(0);
	}
	
	int total_bytes_send = 0;
	while (1) {
		unsigned char buff[MAXDATASIZE];
		memset(buff, '\0', sizeof(buff));
		int bytes_read = fread(buff, sizeof(char), MAXDATASIZE, fp);

		if (bytes_read > 0) {
			printf("Successfully read %d bytes, now send \n", bytes_read);
			if (write(sockfd, buff, bytes_read) == -1){
				printf("Error writing to sockfd %d\n", sockfd);
				exit(0);
			}
			total_bytes_send += bytes_read;
		}

		if (feof(fp)) {
			printf("Reach end of file, total_bytes_send is %d\n", total_bytes_send);
			break;
		}

		if (ferror(fp)) {
			printf("Error reading file, total_bytes_send is %d\n", total_bytes_send);
			exit(0);
		}
	}
	fclose(fp);
	return total_bytes_send;
}


int recv_file(char* filename, int sockfd) {
	FILE* fp = fopen(filename, "a+");
	unsigned char buff[MAXDATASIZE];
	memset(buff, '\0', sizeof(buff));
	int bytes_recv = 0;
	int total_bytes_recv = 0;
	while (1) {
		bytes_recv = read(sockfd, buff, MAXDATASIZE);
		if (bytes_recv < 0) {
			perror("Error reading from sockfd%d\n", sockfd);
			fclose(fp);
			exit(0);
		}
		else if(bytes_recv > 0){
			printf("Successfully receive %d bytes\n", bytes_recv);
			if (fwrite(buff, sizeof(char), bytes_recv, fp) < bytes_recv) {
				perror("fwrite() ends before writing %d (bytes_recv) bytes\n", bytes_recv);
				fclose(fp);
				exit(0);
			}
			total_bytes_recv += bytes_recv;
			fflush(fp);
		}
		else if(bytes_recv == 0){
			//printf ("Reach of file, bytes_recv == 0\n");
			break;
		}
	}

	fclose(fp);
	return total_bytes_recv;
}
