/**
 *	file_transfer.c
 */

#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <unistd.h>

size_t get_filesize (char *filename);

size_t write_to_socket(int sockfd, const char *msg, size_t msg_length);

size_t read_from_socket(int sockfd, const char *msg, size_t msg_length);
/**
 * Send a segment (or say part/ section) of the file specified by filename to sockfd.
 *
 * file_index and file_size are provided by the Scheduling Unit (which schdules what \
 * segment of the file should be sent, and the size of the segment)
 *
 * In case no partition is required (i.e. the entire file is sent instead of a \
 * segment of it), file_index should be set to 0. In this case, file_size also \
 * represents the size of the entire file, instead of its partition's
 *
 * @para 
 *  	filename: 	Name of the file
 *
 *  	sockfd: 	Socket file descriptor
 *
 *  	file_index:  	If it is 0, it means we are sending the entire file,\
 *  	not just a segment of it. Other wise, file_index should start with 1. \
 *  	It marks which segment of the file to be sent.
 *
 *  	file_size: 	Size of the file that will be sent. 
 *
 * @return
 * 	number of bytes that are actually sent.
 *
 * @todo (implementation detail, can be skipped)
 * 	1. Obtain file_size and pad the file_size so that it is divisible by page size of \
 * 	the machine. This is because mmap() can only map data with size divisible by page size.
 *
 * 	2. Using mmap(), map target segment of the file into memory. The result is \
 * 	a memory address containing the data (you can think of it as a char array)
 *
 * 	3. Using write(), write the data into sockfd
 *
 * 	4. Using munmap, unmap the data that was mapped by mmap() previously
 *  
 * 
 */
ssize_t send_file(char *filename, int sockfd, int file_index, size_t file_size);

/**
 * Read data from sockfd and write it to a new file. 
 * Similar to send_file, if file_index is 0 it means an entire file has been sent (no partition)
 * If file_index > 0, we should create a file with indexed file name. \
 * For instance: "alloc1", "alloc2"
 * This is achieved by using sprintf();
 *
 * @para
 * 	base_filename:	Name of the original file (i.e. "alloc", "server")
 *
 * 	sockfd:		Same as send_file()
 *
 * 	file_index:	Same as send_file()
 *
 * 	file_size: 	Same as send_file()
 *
 * @return
 * 	number of bytes actually received.
 *
 * @todo
 * 	1. Using sprintf(), create the indexed file name. Use fopen() to open/create this file
 *
 * 	2. Read data from sockfd using a buffer. Write to the opened file.
 *
 */
ssize_t recv_file(char *base_filename, int sockfd, int file_index, size_t file_size);

 /**
  * Combine all parts of a file into one complete file using mmap()
  * On success, return NULL
  * On failure due to missing segments, return an array of int containing the index of the missing segments
  * This array should be freed after it is no longer of use
  * 
  * @para
  * 	base_filename:	Name of the final combined file. 
  *
  * 	segment_counts:	Total number of partition the file should have. 
  *
  * 	segment_size: 	Size of the each segment.
  *
  * @return
  * 	On success, return NULL
  * 	Else, return an int array of size segment_count containing index of missing segments
  * 	This array should be freed later
  *
  * @todo
  * 	1. Search using access() to make sure all partitions of this file exist
  *
  * 	2. Using fopen(filename, "a"), mmap(), write(), do the combination
  * 	
  */
int* combine_file(char *base_filename, int segment_count);



 /**
  * Return a size_t array of segment_count elements
  * This array is obtained by using malloc(segment_count). It should be freed later.
  * segment[0] - segment[segment_count - 2] contains the size of each regular segment
  * segment[segment_count - 1] contains the size of the last segment, which might be different\
  * from other regular segments due to padding.
  *
  * Mmap() can only map data of size divisible by page_size. If the original file_size \
  * is not divisible by page_size, we need to pad the original file. The actual \
  * padding of file (using ftruncate()) is done in send_file. Here we just calculate the \
  * segment size.
  *
  * For instance, if the original file_size is 10 pages, and segment_count is 3:
  * segment[0] = segment[1] = 3 * page; segment[2] = 4 * page
  *
  * @para
  * 	file_size:	Size of the entire file
  *
  * 	segment_count:	How many segment this file should have
  *
  * @return
  * 	Return an arrary containing segment_size based on given para
  * 	This array should be freed later
  *
  * @todo
  * 	1. Calculate a new file_size that is divisible by page_size
  *
  * 	2. Calculate segment_size of every segment (last segment might be larger than others)
  * 	
  *
  */
void schedule_segment_size (size_t *segments, size_t file_size, int segment_count);


#endif
