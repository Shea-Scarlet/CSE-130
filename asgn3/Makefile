CC = clang
CFLAGS = -Wall -Wextra -Werror -pedantic -pthread

all: queue.o

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

clean:
	rm -f queue.o
