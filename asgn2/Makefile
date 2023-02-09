CC = clang
CFLAGS = -Wall -Wextra -Werror -pedantic

all: httpserver

httpserver: httpserver.o asgn2_helper_funcs.a
	$(CC) httpserver.o asgn2_helper_funcs.a -o httpserver -l m

httpserver.o: httpserver.c asgn2_helper_funcs.h
	$(CC) $(CFLAGS) -c httpserver.c 

clean:
	rm -f *.o httpserver
