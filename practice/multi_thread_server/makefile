src=$(wildcard *.c)
obj=$(patsubst %.c, %.o, $(src))

CC=gcc
CFLAGS= -Wall -g -lpthread

all:server client

server:server.o wrap.o threadpool.o
	$(CC) $^ -o $@ $(CFLAGS)

client:client.o wrap.o
	$(CC) $^ -o $@ $(CFLAGS)

$(obj):%.o:%.c
	$(CC) -c $< $(CFLAGS)

.PHONY:clean all
clean:
	rm -rf $(obj) server client
