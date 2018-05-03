/**
 *	file_transfer.c
 */

#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <unistd.h>

// wrapper function for using stat() to get file size
size_t get_filesize (char *filename);


// wrapper function to write to/ read from socket into a preallocated buffer char *msg \
// the functions handle errno == EINTER
size_t write_to_socket(int sockfd, const char *msg, size_t msg_length);

size_t read_from_socket(int sockfd, const char *msg, size_t msg_length);


/**
 * Send a segment (i.e. part/ section) of the file specified by filename to sockfd.
 *
 * file_index,  file_size, reg_segment_size are provided by the function schedule_segment_size() (which \
 * schdules what segment of the file should be sent, and the size of the segment).
 *
 * In case no partition is required (i.e. the entire file is sent instead of a \
 * segment of it), file_index should be set to 0. In this case, file_size also \
 * represents the size of the entire file, instead of its partition's
 *
 * @para
 *  	filename: 		Name of the file
 *
 *  	sockfd: 		Socket file descriptor
 *
 *  	file_index:  		If it is 0, it means we are sending the entire file,\
 *  	not just a segment of it. Other wise, file_index should start with 1. \
 *  	It marks which segment of the file to be sent.
 *
 *  	file_size: 		Size of the data (segment/ entire file) that will be sent.
 *
 *  	reg_segment_size: 	Size of regular segments. For regular segments (every \
 *      segment before the last one), file_size == reg_segment_size. The last segment \
 *      might have a size different from previous regular segments. We need to know size \
 *      of regular segments to calculate the starting addr of the last segment. Read \
 *      schedule_segment_size() for more info.
 *
 * @return
 * 	Number of bytes that are actually sent. Corresponding error message should be printed \
 *      when the bytes_sent != file_size
 *
 * @todo (implementation detail, can be skipped)
 *
 * 	1. Using mmap(), map target segment of the file into memory. Calculate the starting addr \
 *      of the segment we want to send (in case of sending an entire file, the starting addr is \
 *      simply the return value of mmap())
 *
 * 	2. Using write(), write the segment into sockfd
 *
 * 	3. Using munmap, unmap the data that was mapped by mmap() previously
 *
 *
 */
size_t send_file(char *filename, int sockfd, size_t index, size_t file_size, size_t reg_segment_size);

/**
 * Read data from sockfd and write it to a new file.
 *
 * Similar to send_file, if file_index is 0 it means an entire file has been sent (no partition)
 *
 *
 * If file_index > 0, we should create a file with indexed file name. i.e. "myfile1.c", "myfile2.c"
 * This is achieved by using sprintf();
 *
 * @para
 * 	base_filename:	Name of the original file (i.e. "myfile.c", "server.c")
 *
 * 	sockfd:		Same as send_file()
 *
 * 	index:	        Same as send_file()
 *
 * 	file_size: 	Same as send_file()
 *
 * @return
 * 	Number of bytes actually received. Print error message accordingly
 *
 * @todo
 * 	1. Using sprintf(), create the name the segment. Use fopen() to open/create this file
 *
 * 	2. Read data from sockfd using a buffer. Write to the opened file.
 *
 */
size_t recv_file(char *base_filename, int sockfd, size_t index, size_t file_size);

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
