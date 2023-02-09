#include <assert.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "asgn2_helper_funcs.h"

#define BUFFER_SIZE 4096

struct my_parsing
{
	char* filename;
	char* content_length;
	char* body;
};

int main(int argc, char **argv)
{

	struct my_parsing cmd;
	char* port_number = argv[1];
	
	if (argc < 2 || !isdigit(*port_number))
	{
		//print_usage(argv[0]);
		return 1;
	}

	if (atoi(port_number) < 1 || atoi(port_number) > 65535)
	{
		fprintf(stderr, "Invalid Command\n");
		return 1;
	}
	
	//creating a socket
	
	Listener_Socket my_socket;
	int my_port = listener_init(&my_socket, atoi(port_number));
	
	
	if (my_port != 0)
	{
		fprintf(stderr, "Could not create socket\n");
		return 1;
	}
	
	//regex
	
	regex_t regex_get, regex_put, regex_location, regex_content_length, regex_body;
	regmatch_t matches[1], matches2[1], matches3[1];
	
	regcomp(&regex_get, "GET", 0);
	regcomp(&regex_put, "PUT", 0);
	regcomp(&regex_location, "/[^/]*\\.txt", 0);
	regcomp(&regex_content_length, "Content-Length: [0-9]+", REG_EXTENDED | REG_NEWLINE);
	regcomp(&regex_body, "\r\n\r\n(.*)", REG_NEWLINE);
	
	//forever
	
	for  (;;)
	{
		//creating a buffer
			
		char cmd_buffer[BUFFER_SIZE];
		
		int accept_socket = listener_accept(&my_socket);
		read_until(accept_socket, cmd_buffer, sizeof cmd_buffer, NULL);
		
		//int get_test = regexec(&regex_get, cmd_buffer, 0, NULL, 0);
		int put_test = regexec(&regex_put, cmd_buffer, 0, NULL, 0);
		int location = regexec(&regex_location, cmd_buffer, 1, matches, 0);
		int length = regexec(&regex_content_length, cmd_buffer, 1, matches2, 0);
		int body = regexec(&regex_body, cmd_buffer, 1, matches3, 0);

		printf ("%d\n", body);
		
		//(put in function)
		
		if (put_test == 0 && location != 0)
		{
			//print_usage(argv[0]);
			return 1;
		}
		else
		{
			int start = matches[0].rm_so;
			int end = matches[0].rm_eo;
			int l = end - start;
			char m_string[100];
			strncpy(m_string, cmd_buffer + start, l);
			m_string[l] = '\0';
			cmd.filename = m_string;
			printf("%s\n", cmd.filename);
		}
		
		if (put_test == 0 && length !=0)
		{
			//print_usage(argv[0]);
			return 1;
		}
		else
		{
			int start = matches2[0].rm_so;
			int end = matches2[0].rm_eo;
			int l = end - start;
			char m_string[100];
			strncpy(m_string, cmd_buffer + start, l);
			m_string[l] = '\0';
			cmd.content_length = m_string;
			printf("%s\n", cmd.content_length);
		}
		
		if (put_test == 0 && body !=0)
		{
			//print_usage(argv[0]);
			return 1;
		}
		else
		{
			int start = matches3[0].rm_so;
			int end = matches3[0].rm_eo;
			int l = end - start;
			char m_string[100];
			strncpy(m_string, cmd_buffer + start, l);
			m_string[l] = '\0';
			cmd.body = m_string;
			printf("%s\n", cmd.body);
		}
		
	}
	
	
  	return 0;

	
}
		

