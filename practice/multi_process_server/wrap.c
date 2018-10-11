#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

void perr_exit(const char* s) {
    perror(s);
    exit(1);
}

int Socket(int family, int type, int protocol) {
    int n;
    if((n = socket(family, type, protocol)) < 0) 
        perr_exit("socket error");
    return n;
}

int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr) {
    int n;
again:
    if((n = accept(fd, sa, salenptr)) < 0) {
        if(errno == ECONNABORTED || errno == EINTR)
            goto again;
        else 
            perr_exit("accept error");
    }
    return n;
}

int Bind(int fd, const struct sockaddr* sa, socklen_t salen) {
    int n;
    if((n = bind(fd, sa, salen)) < 0)
        perr_exit("bind error");
    return n;
}

int Connect(int fd, const struct sockaddr* sa, socklen_t salen) {
    int n;
    if((n = connect(fd, sa, salen)) < 0)
        perr_exit("connect error");
    return n;
}

int Listen(int fd, int size) {
    int n;
    if((n = listen(fd, size)) < 0)
        perr_exit("listen error");
    return n;
}

ssize_t Read(int fd, void* ptr, size_t nbytes) {
    size_t n;
again:
    if( (n = read(fd, ptr, nbytes)) < 0 ) {
        if(errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}

ssize_t Write(int fd, void* ptr, size_t nbytes) {
    size_t n;
again:
    if( (n = write(fd, ptr, nbytes)) < 0 ) {
        if(errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}

int Close(int fd) {
    int n;
    if( (n = close(fd)) < 0 )
        perr_exit("close error");
    return n;
}