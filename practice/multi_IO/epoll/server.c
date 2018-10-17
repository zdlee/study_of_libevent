#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 10
#define SERVER_PORT 7777

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    int listenfd, connfd, efd;
    socklen_t cliaddr_len;
    int ready, len;
    char buf[BUFSIZ], str[BUFSIZ];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    bind(listenfd, (struct sockaddr*) &server_addr, sizeof(server_addr));

    listen(listenfd, 1024);

    struct epoll_event event;
    struct epoll_event reevent[10];

    printf("Accepting connections ...\n");

    cliaddr_len = sizeof(client_addr);
    connfd = accept(listenfd, (struct sockaddr *)&client_addr, &cliaddr_len);
    printf("received from %s at PORT %d\n",
            inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
            ntohs(client_addr.sin_port));

    efd = epoll_create(10);
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = connfd;
    epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);

    while(1) {
        ready = epoll_wait(efd, reevent, 10, -1);
        if(ready < 0) {
            printf("epoll_wait failed\n");
            //exit(1);
        }
        if(reevent[0].data.fd == connfd) {
            len = read(connfd, buf, MAXLINE/2);
            if(len == 0) {
                printf("the other side has closed\n");
                close(connfd);
            }
            write(STDOUT_FILENO, buf, len);
        }
    }
    close(connfd);
    close(listenfd);

    return 0;
}

