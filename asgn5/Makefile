CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic
LDFLAGS = -lm
EXEC = cacher
OBJS = cacher.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS)

cacher.o: cacher.c
	$(CC) $(CFLAGS) -c cacher.c

clean:
	rm -f $(EXEC) $(OBJS)
