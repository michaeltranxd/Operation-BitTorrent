
#include "file_transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>

//#define SEGMENT_COUNT 4

#define MAXDATASIZE 256	// max number of bytes we can get at once
// consider increasing the size limit, read https://en.wikipedia.org/wiki/Maximum_transmission_unit

static long file_size(char *filename) {
	FILE *fp = fopen(filename, "rb");
	if (fp == -1) {
		printf("fopen()\n");
		return -1;
	}

	long size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp) + 1;
	fclose(fp);

	return size;

}



ssize_t send_file(char* filename, int sockfd, int file_index, int file_size){
	if (file_index == 0) {
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

	else {
		int fd = open(filename, O_RDONLY);
		if (fd == -1) {
			printf("Error open() file %s\n", filename);
			exit(0);
		}
		struct stat s;
		fstat(fd, &s);
		size_t old_size = s.st_size; // original file size of fd
		
		size_t extra_size = 0;
		if (old_size % file_size != 0) extra_size = file_size - (old_size % file_size);
		size_t new_size = old_size + extra_size; // new file size of fd
		assert((new_size % file_size) == 0); // make sure new_size is divisible by file_size
		ftruncate(fd, new_size); // this will append NULL bytes to the end of fd until its size becomes fd_new_size


		size_t offset = (file_index - 1) * file_size;
		void *addr0 = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (addr0 == MAP_FAILED) {
			printf("Error mapping file descriptor %d\n", fd);
			exit(0);
		}
		void *addr_at_index = (void *)((char *)addr0 + offset);

		ssize_t total_bytes_read = write(sockfd, addr_at_index, file_size);
		if (total_bytes_read == -1) {
			printf("Error writing to sockfd %d\n", sockfd);
			exit(0);
		}
		if (munmap(addr0, new_size) == -1) {
			printf("Error unmapping\n");
		}

		close(fd);
		return total_bytes_read;
	}
		
}


ssize_t recv_file(char* base_filename, int sockfd, int file_index, size_t file_size) {
	FILE *fp;
	if (file_index != 0) {
		char filename[256];
		sprintf(filename, "%s%d", base_filename, file_index);
		fp = fopen(filename, "w+");
	}
	else {
		fp = fopen(base_filename, "a+");
	}

	if (fp == -1) {
		printf("Error fopen() base_filename is %s\n", base_filename);
	}

	unsigned char buff[MAXDATASIZE];
	memset(buff, '\0', sizeof(buff));
	ssize_t bytes_recv = 0;
	ssize_t total_bytes_recv = 0;
	while (1) {
		bytes_recv = read(sockfd, buff, MAXDATASIZE);
		if (bytes_recv < 0) {
			printf("Error reading from sockfd%d\n", sockfd);
			fclose(fp);
			exit(0);
		}
		else if(bytes_recv > 0){
			printf("Successfully receive %d bytes\n", bytes_recv);
			if (fwrite(buff, sizeof(char), bytes_recv, fp) < (unsigned)bytes_recv) {
				printf("fwrite() ends before writing %d (bytes_recv) bytes\n", bytes_recv);
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

int* combine_file(char *base_filename, int segment_count) {
	if (segment_count < 2) return NULL;

	int *ret = (int *) malloc(segment_count * sizeof(int));
	int ret_itr = 0;

	int index = 1;
	void *addr[segment_count];
	size_t segment[segment_count]; // records the size of each segment
	while (index < (segment_count + 1)) {
		char indexed_filename[256];
		sprintf(indexed_filename, "%s%d", base_filename, index);
		if ( !access(indexed_filename, F_OK) ) {
			ret[ret_itr] = index;
			ret_itr ++;
			addr[index] = NULL;
		}
		else {
			int fd = open(indexed_filename, "r");
			if (fd == -1) {
				printf("Error open() indexed_filename %s\n", indexed_filename);
				close(fd);
				free(ret);
				exit(0);
			}
			struct stat s;
			fstat(fd, &s);
			size_t segment_size = s.st_size;
			segment[index] = segment_size;

			addr[index] = mmap(NULL, segment_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (addr[index] == MAP_FAILED) {
				printf("Error mmap() fd %d\n", fd);
				close(fd);
				free(ret);
				exit(0);
			}

			close(fd);
		}
	}

	if (ret[0] != NULL) { // at least one segment is missing
		index = 1;
		while (index < (segment_count + 1)) {
			if (addr[index] != NULL) {
				if (munmap(addr[index], segment[index]) == -1) {
					printf("Error munmap() addr[%d]\n", index);
				}
			}
		}
		return ret;
	}
	else { // combine all segments
		int fd_combined = open(base_filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
		if (fd_combined == -1) {
			printf("Error open() base_filename %s\n", base_filename);
			free(ret);
			exit(0);
		}
		FILE *fp_combined = fdopen(fd_combined, "a");
		if (fp_combined == NULL) {
			printf("Error fdopen() fd_combined %d\n", fd_combined);
			free(ret);
			close(fd_combined);
			exit(0);
		}
		index = 1;
		while (index < (segment_count + 1)) {
			if (write(fp_combined, addr[index], segment[index] == -1) {
				printf("Error write() to fp_combined %d, index is %d\n", base_filename, index);
				free(ret);
				close(fd_combined);
				fclose(fp_combined);
				exit(0);
			} 
			if (munmap(addr[index], segment[index]) == -1) {
				printf("Error munmap() addr[%d]\n", index);
			}
		}

		close(fd_combined);
		fclose(fp_combined);
		free(ret);
		return NULL;

	}
	
}

size_t* schedule_segment_size (size_t file_size, int segment_count) {
	size_t *segment = (size_t *) malloc((segment_count + 1) * sizeof(size_t));

	size_t old_size = file_size;
	size_t page_size = (size_t) sysconf(_SC_PAGESIZE);
	size_t extra_size = 0;
	if (old_size % page_size != 0) {
		extra_size = page_size - (old_size % page_size);
	}
	size_t new_size = old_size + extra_size;

	int page_count = new_size / page_size;
	size_t regular_segment = (page_count / segment_count) * page_size;
	size_t last_segment = ((page_count / segment_count) + 1) * page_size;

	int itr = 1;
	segment[0] = 0;
	while (itr < (segment_count + 1)) {
		segment[itr] = regular_segment;
		itr ++;
	}
	segment[itr] = last_segment;
	return segment;
}

