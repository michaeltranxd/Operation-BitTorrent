
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
#include <errno.h>

//#define SEGMENT_COUNT 4

#define MAXDATASIZE 256	// max number of bytes we can get at once
// consider increasing the size limit, read https://en.wikipedia.org/wiki/Maximum_transmission_unit

size_t get_filesize(char *filename) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Error fopen() file %s\n", filename);
		exit(-1);
	}

	long size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp) + 1;
	fclose(fp);

	return (size_t)size;
}



ssize_t send_file(char* filename, int sockfd, size_t index, size_t filesize){
	if (index == 0) {  // sending the whole file instead of a partition of it
		FILE* fp = fopen(filename, "r");
		if (!fp) {
			printf("Fail to open file %s\n", filename);
			exit(-1);
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
					exit(-1);
				}
				total_bytes_send += bytes_read;
			}

			if (feof(fp)) {
				printf("Reach end of file, total_bytes_send is %d\n", total_bytes_send);
				break;
			}

			if (ferror(fp)) {
				printf("Error reading file, total_bytes_send is %d\n", total_bytes_send);
				exit(-1);
			}
		
		
		}
		
		fclose(fp);
		return total_bytes_send;
	}

	else {
		int fd = open(filename, O_RDONLY);
		if (fd == -1) {
			printf("Error open() file %s\n", filename);
			exit(-1);
		}
		struct stat s;
		fstat(fd, &s);
		size_t old_size = s.st_size; // original file size of fd
		
		size_t page_size = (size_t) sysconf(_SC_PAGESIZE);
		size_t extra_size = 0;
		if (old_size % page_size != 0) extra_size = page_size - (old_size % page_size);
		size_t new_size = old_size + extra_size; // new file size of fd
		assert((new_size % page_size) == 0); // make sure new_size is divisible by page_size
		if (ftruncate(fd, new_size) == -1) { // this will append NULL bytes to the end of fd until its size becomes new_size
			printf("Error ftruncate() file %s to new_size %zu\n", filename, new_size);
			exit(-1);
		}

		size_t offset = (index - 1) * page_size;
		void *addr0 = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (addr0 == MAP_FAILED) {
			printf("Error mmap() file %s\n", filename);
			exit(-1);
		}
		void *addr_at_index = (void *)((char *)addr0 + offset);

		ssize_t total_bytes_write = write(sockfd, addr_at_index, filesize);
		if (total_bytes_write == -1) {
			printf("Error writing to sockfd %d\n", sockfd);
			// exit(-1);
		}
		if (munmap(addr0, new_size) == -1) {
			printf("Error unmapping\n");
		}

		close(fd);
		return total_bytes_write;
	}
		
}


ssize_t recv_file(char* base_filename, int sockfd, size_t index, size_t filesize) {
	int fd;
	FILE *fp;
	char *filename = (char *) calloc(256, sizeof(char));
	if (index != 0) { // a segment for the file (named as base_filename) should be received
		sprintf(filename, "%s%zu", base_filename, index);
	}
	else { // entire file should be received
		memcpy(filename, base_filename, strlen(base_filename));
	}

	if ((fd = open(filename, O_CREAT | O_TRUNC | O_RDWR)) == -1) {
		printf("Error open() file %s\n", filename);
		exit(-1);
	}
	if ((fp = fdopen(fd, "w+")) == NULL) {
		printf("Error fdopen() file %s, fd %d\n", filename, fd);
		exit(-1);
	}
	free(filename);

	unsigned char buff[MAXDATASIZE];
	memset(buff, '\0', sizeof(buff));
	ssize_t bytes_recv = 0;
	ssize_t total_bytes_recv = 0;
	while (1) {
		bytes_recv = read(sockfd, buff, MAXDATASIZE);
		if (bytes_recv < 0) {
			if (errno == EINTR) {}
			else {
				printf("Error reading from sockfd%d\n", sockfd);
				fclose(fp);
				exit(-1);
			}
		}
		else if(bytes_recv > 0){
			//printf("Successfully receive %d bytes\n", bytes_recv);
			if (fwrite(buff, sizeof(char), bytes_recv, fp) < (unsigned)bytes_recv) {
				printf("fwrite() ends before writing %zu (bytes_recv) bytes\n", bytes_recv);
				fclose(fp);
				exit(-1);
			}
			total_bytes_recv += bytes_recv;
			fflush(fp);
		}
		else if(bytes_recv == 0){
			if (total_bytes_recv == filesize) break;
			//printf ("Reach of file, bytes_recv == 0\n");
			else {
				printf("Reading less data than supposed to, total_bytes_recv %zu, filesize %zu\n", total_bytes_recv, filesize);
			}
		}
	}

	fclose(fp);
	return total_bytes_recv;
}


int* combine_file(char *base_filename, int segment_count) {
	if (segment_count < 2) return NULL; // there should be at least 2 segments

	int *ret = (int *) malloc((segment_count + 1) * sizeof(int)); // last element will be 0, for end array detection
	int ret_itr = 0;

	void **addr = (void **) malloc((segment_count + 1) * sizeof(void *)); // record the addr each segment is mapped to
	size_t segment[segment_count + 1]; // records the size of each segment

	
	int index = 1;
	while (index < (segment_count + 1)) {
		char indexed_filename[256];
	//	char indexed_filename_path[256];
		sprintf(indexed_filename, "%s%d", base_filename, index);
	//	sprintf(indexed_filename_path, "~/stuff/%s", indexed_filename);

	//	if ( !access(indexed_filename_path, R_OK) ) {
	//		ret[ret_itr] = index;
	//		ret_itr ++;
	//		addr[index] = NULL;
	//		printf("the segment index that is missing on disk is %d\n", index);
	//	}
	//	else {
			int fd = open(indexed_filename, O_RDWR, S_IRWXU);
			if (fd == -1) {
				printf("Error open() indexed_filename %s\n", indexed_filename);
				printf("%s\n", strerror(errno));

				ret[ret_itr] = index;
				ret_itr ++;
				addr[index] = NULL;
				printf("the segment index related to this file is %d\n", index);

				close(fd);
			}

			else {
				struct stat s;
				fstat(fd, &s);
				size_t segment_size = s.st_size;
				segment[index] = segment_size;

				addr[index] = mmap(NULL, segment_size, PROT_READ, MAP_SHARED, fd, 0);
				//printf("segment index %d, addr[%d] %p\n", index, index, addr[index]);
				if (addr[index] == MAP_FAILED) {
					printf("Error mmap() fd %d\n", fd);
					printf("the segment index related to this fd is %d\n", index);
					close(fd);
					free(ret);
					exit(-1);
				}

				close(fd);
			}
	//	}

		index ++;
	}
	ret[ret_itr] = 0; // set last element to 0

	if (ret[0] != 0) { // at least one segment is missing. Return ret (and request the missing segments)
		printf("At least one segment is missing, since ret is non-empty\n");
		index = 1;
		while (index < (segment_count + 1)) {
			if (addr[index] != NULL) {
				if (munmap(addr[index], segment[index]) == -1) {
					printf("Error munmap() addr[%d]\n", index);
				}
			}
			index ++;
		}
		return ret;
	}

	else { // combine all segments
		free(ret); // no segment is missing, so we don't need to return ret.
		int fd_combined = open(base_filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
		if (fd_combined == -1) {
			printf("Error open() base_filename %s\n", base_filename);
			exit(-1);
		}
		FILE *fp_combined = fdopen(fd_combined, "a");
		if (fp_combined == NULL) {
			printf("Error fdopen() fd_combined %d\n", fd_combined);
			close(fd_combined);
			exit(-1);
		}

		index = 1;

		while (index < (segment_count + 1)) {
			//printf("segment index %d, addr[index] %p\n", index, addr[index]);
			if (fwrite(addr[index], sizeof(char), segment[index], fp_combined) < segment[index]) {
				printf("fwrite() ends before writing %zu bytes to file %s from segment indexed %d\n", segment[index], base_filename, index);
				printf("%s\n", strerror(errno));
				free(ret);
				fclose(fp_combined);
				exit(-1);
			} 
			if (munmap(addr[index], segment[index]) == -1) {
				printf("Error munmap() addr[%d]\n", index);
			}
			index ++;
		}

		fclose(fp_combined);
		free(ret);
		return NULL;

	}
}

void schedule_segment_size (size_t *segments, size_t filesize, int segment_count) {
	size_t old_size = filesize;
	size_t page_size = (size_t) sysconf(_SC_PAGESIZE);
	size_t extra_size = 0;
	if (old_size % page_size != 0) {
		extra_size = page_size - (old_size % page_size);
	}
	size_t new_size = old_size + extra_size;

	int page_count = new_size / page_size;
	size_t regular_segment = (page_count / segment_count) * page_size;
	size_t last_segment = ((page_count % segment_count) + (page_count / segment_count)) * page_size;

	int itr = 0;
	while (itr < segment_count - 1) {
		segments[itr] = regular_segment;
		itr ++;
	}
	segments[itr] = last_segment;
	return;
}

