CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic
LDFLAGS = -lm
EXEC = memory
OBJS = memory.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS)

memory.o: memory.c
	$(CC) $(CFLAGS) -c memory.c

clean:
	rm -f $(EXEC) $(OBJS)
