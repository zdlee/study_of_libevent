#include <event.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_PORT 7777

void sock_accept_cb(int fd, short event, void* argv);
void sock_read_cb(int fd, short event, void* argv);
int tcp_server_init(short port);

int main() {
    int listenfd;

    listenfd = tcp_server_init(SERVER_PORT);

    struct event_base* base = event_base_new();

    struct event* ev_listen = event_new(base, listenfd, EV_READ | EV_PERSIST, sock_accept_cb, base); 

    event_add(ev_listen, NULL);

    event_base_dispatch(base);

    return 0;
}

int tcp_server_init(short port) {
    int sockfd, opt, flag;
    struct sockaddr_in server_addr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("socket failed\n");
        exit(1);
    }

    len = sizeof(server_addr);
    bzero(&server_addr, len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    flag = bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(flag < 0) {
        perror("bind failed\n");
        exit(1);
    }

    flag = listen(sockfd, 1024);
     if(flag < 0) {
        perror("listen failed\n");
        exit(1);
    }

    printf("waiting for connect...\n");
    return sockfd;
}

void sock_accept_cb(int fd, short event, void* argv) {
    struct sockadd_in* client_addr;
    socklen_t client_addr_len;
    struct event_base* base = (struct event_base*) argv;

    client_addr_len = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr*) &client_addr, &client_addr_len);
    if(connfd < 0) {
        perror("connect failed\n");
        exit(1);
    }

    struct event* ev_client = event_new(NULL, -1, 0, NULL, NULL);
    event_assign(ev_client, base, connfd, EV_READ | EV_PERSIST, sock_read_cb, (void*) ev_client);

    event_add(ev_client, NULL);
}

void sock_read_cb(int fd, short event, void* argv) {
    int n, i;
    char buf[BUFSIZ];

    n = read(fd, buf, BUFSIZ);
    if(n < 0) {
        perror("read failed\n");
        exit(1);
    } else if(n == 0) {
        printf("the other side has been closed\n");
        close(fd);
    }

    write(STDOUT_FILENO, buf, n);
    for(i = 0; i < n; i++) {
        buf[i] = toupper(buf[i]);
    }
    write(fd, buf, n);
}