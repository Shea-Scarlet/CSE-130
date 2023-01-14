CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic
LDFLAGS = -lm
EXEC = hello
OBJS = hello.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS)

hello.o: hello.c
	$(CC) $(CFLAGS) -c hello.c

clean:
	rm -f $(EXEC) $(OBJS)
