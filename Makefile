# compiler: gcc
CC = gcc

# compiler flags:
CFLAGS  = -g -Wall

all: server client

server: src/server.c src/db.c
	$(CC) $(CFLAGS) -o bin/server src/server.c src/db.c -lpthread

client: src/client.c
	$(CC) $(CFLAGS) -o bin/client src/client.c

clean:
	rm -rf bin/*