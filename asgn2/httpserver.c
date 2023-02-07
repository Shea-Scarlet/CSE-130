#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{

	//creating a socket
	
	Listener_Socket my_socket;
	
	if (my_socket == -1)
	{
		printf("Could not create socket");
		return 1;
	}
	
	if (argc < 2)
	{
		print_usage(argv[0]);
		return 1;
	}

	char* port = argv[1];

	if (port < 1 || port > 65535)
	{
		fprintf(stderr, "Invalid Command\n");
		return 1;
	}
	
	for  (;;)
	{
		int success = listener_init(sock, atoi(port));
		int accept_socket = listener_accept(my_socket)
		
	}
	
	
	

}
		

