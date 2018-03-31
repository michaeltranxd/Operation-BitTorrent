/**
 *	file_transfer.c
 */

#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

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
 *  	file_size: 	Size of the file that will be sent (The original \
 *  	file can be partitioned into many segments with the same size. file_size \
 *  	specifies this size)
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
ssize_t send_file(char *filename, int sockfd, int file_index, int file_size);

/**
 * Read data from sockfd and write it to a new file. 
 * Similar to send_file, if file_index is 0 it means an entire file has been sent (no partition)
 * If file_index > 0, we should create a file with indexed file name. \
 * For instance: "alloc1", "alloc2", "server2"\
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
 * 
 *
 *	
 */
ssize_t recv_file(char *base_filename, int sockfd, int file_index, size_t file_size);

 /**
  * Combine all parts of a file into one complete file using mmap()
  * 
  * @para
  * 	base_filename:	Name of the final combined file. Search in log file to find \
  * 	its partitions/ segments
  *
  * @todo
  * 	1. Search within log file to make sure all partitions of this file exist
  *
  * 	2. Using fopen(filename, "a"), mmap(), write(), do the combination
  * 	
  */
void combine_file(char *base_filename);



 /**
  * 
  * Search for target_filename within the log file specified by log_filename
  * Related log_file functions (i.e. add, delete) should also be implemented before hand 
  *
  * @para
  * 	log_filename:		Name of the log file
  *
  * 	target_filename:	Name of the target file. 
  *
  * 	total_parts:		The number of segments that the target file should have. \
  * 	This information is provided by the Scheduling Unit. For instance, if the \
  * 	Scheduling Unit divides a file into 5 segments, then total_parts should be 5. 
  * 	This can be used to check if there is a missing segment of the target file.
  *
  * @return
  *	(I dont have a solid idea of what to return right now, work it out later)
  *	(Maybe write the path of each patition into an output file)
  *
  */
int log_file_search(char *log_filename, char *target_filename, FILE *output_fp, int total_segments)

#endif
