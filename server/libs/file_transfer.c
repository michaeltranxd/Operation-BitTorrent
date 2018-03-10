
#include "file_transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int send_file(char* filename, int new_fd){
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		printf("Fail to open file %s\n", filename);
		return -1;
	}
	
	int total_bytes_send = 0;
	while (1) {
		unsigned char buff[256] = {0};
		memset(buff, '\0', sizeof(buff));
		int bytes_read = fread(buff, sizeof(char), 256, fp);

		if (bytes_read > 0) {
			printf("Successfully read %d bytes, now send \n", bytes_read);
			if (write(new_fd, buff, bytes_read) == -1)
				printf("Error writing to new_fd %d\n", new_fd);
			total_bytes_send += bytes_read;
		}

		if (feof(fp)) {
			printf("Reach end of file, total_bytes_send is %d\n", total_bytes_send);
			break;
		}

		if (ferror(fp)) {
			printf("Error reading file, total_bytes_send is %d\n", total_bytes_send);
			break;
		}
	}
	fclose(fp);
	return total_bytes_send;
}



int recv_file(char* filename, int sockfd) {
	FILE* fp = fopen(filename, "a+");
	unsigned char buff[256];
	memset(buff, '\0', sizeof(buff));
	int bytes_recv = 0;
	int total_bytes_recv = 0;
	while (1) {
		bytes_recv = read(sockfd, buff, 256);
		if (bytes_recv < 0) {
			printf("Read error\n");
			break;
		}
		else if(bytes_recv > 0){
			printf("Successfully receive %d bytes\n", bytes_recv);
			printf("string: %s \n", buff);
			fwrite(buff, sizeof(char), bytes_recv, fp);
			fflush(fp);
			total_bytes_recv += bytes_recv;
		}
	}
	fclose(fp);
	return total_bytes_recv;
}
