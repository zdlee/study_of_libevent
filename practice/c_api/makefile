src=$(wildcard *.c)
obj=$(patsubst %.c, %.o, $(src))

CC=gcc
CFLAGS= -Wall -g

all:server client

server:server.o wrap.o
	$(CC) $^ -o $@ $(CFLAGS)

client:client.o wrap.o
	$(CC) $^ -o $@ $(CFLAGS)

$(obj):%.o:%.c
	$(CC) -c $< $(CFLAGS)

.PHONY:clean all
clean:
	rm -rf $(obj) server client
