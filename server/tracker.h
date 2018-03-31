/**
 *	server_client.h
 *
 */

/**
 *	This method is the threaded method for
 *	running the server in parallel. This is
 *	to have the server running in parallel
 *	with the client
 *
 * 	args:
 * 			ptr is a pointer to the argc_argv (argcv) 
 * 			struct because it gives the server argc 
 * 			and argv (to start the server)
 *
 */
void* server_thread_method(void* ptr);

/**
 *	This method is the threaded method for
 *	running the multiple clients in parallel.
 *	This will make it so that we will be able
 *	to download multiple parts without waiting
 *	on a single part.
 *
 *	args:
 *			ptr is a pointer to the client_args (c_args)
 *			struct so that the client can know the hostname
 *			and the port to connect to (from user input)
 *
 */
void* multi_client(void* ptr);

/**
 *	This method is the threaded method for
 *	running the client in parallel. This is
 *	to have the client running in parallel
 *	with the server
 *
 *	args:
 *			ptr is a pointer (ALWAYS NULL). Never used
 *			because this is to set up the client thread
 *			so that it can continuously receive user
 *			input of hostname and port while running
 *			in parallel.
 */
void* client_thread_method(void* ptr);


