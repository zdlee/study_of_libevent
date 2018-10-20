src=$(wildcard *.c)
obj=$(patsubst %.c, %.o, $(src))

CC=gcc
CFLAGS=-Wall -g -levent

all:server client

server:server.o
	$(CC) $^ -o $@ $(CFLAGS)

client:client.o
	$(CC) $^ -o $@ $(CFLAGS)

$(obj):%.o:%.c
	$(CC) -c $< $(CFLAGS)

.PHONY:all clean
clean:
	rm -rf $(obj) server client