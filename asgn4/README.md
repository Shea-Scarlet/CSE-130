#Assignment 4 directory

This directory contains source code and other files for Assignment 4.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

This is a C program that implements a simple HTTP server. It listens for incoming connections on a specified port and handles GET and PUT requests. The program uses multiple threads to handle incoming connections and can handle multiple clients simultaneously.
The program uses several header files and source files including "asgn2_helper_funcs.h", "connection.h", "debug.h", "response.h", "request.h", and "queue.h". It also includes several standard C libraries such as <err.h>, <errno.h>, <fcntl.h>, <signal.h>, <stdlib.h>, <stdio.h>, <string.h>, <unistd.h>, <stdbool.h>, <pthread.h>, <getopt.h>, <sys/socket.h>, <arpa/inet.h>, and <sys/stat.h>.
The program defines several constants such as "DEFAULT_THREADS" which is the default number of threads, "QUEUE_SIZE" which is the maximum size of the queue of connections, and "BUFSIZE" which is the size of the buffer used to read incoming data from clients.
The program defines a "log_lock" mutex for locking file writes and a "stop_server" flag for indicating if the server should stop accepting connections.
The program defines several functions such as "usage" which prints the correct usage of the program, "log_entry" which logs information about a request, "handle_get" which handles GET requests, "handle_unsupported" which handles unsupported requests, "handle_put" which handles PUT requests, and "handle_signal" which handles signals.
The "handle_request" function reads incoming data from a client, creates a connection object, and handles the request by calling the appropriate handler function based on the HTTP method. If the server is stopping, the function returns without processing the request.
Overall, the program provides a basic implementation of a multithreaded HTTP server that can handle GET and PUT requests. However, it lacks many important features such as security measures, error handling, and support for other HTTP methods.

