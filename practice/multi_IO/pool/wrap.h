#ifndef _WRAP_H_
#define _WRAP_H_

#include <sys/socket.h>
#include <sys/types.h>

void perr_exit(const char* s);

int Socket(int family, int type, int protocol);

int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr);

int Bind(int fd, const struct sockaddr* sa,socklen_t salen);

int Connect(int fd, const struct sockaddr* sa, socklen_t salen);

int Listen(int fd, int size);

ssize_t Read(int fd, void* ptr, size_t nbytes);

ssize_t Write(int fd, const void* ptr, size_t nbytes);

int Close(int fd);


#endif