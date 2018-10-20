#include <event.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_PORT 7777

int client_connect_server(short server_port);
void cmd_read_cb(int fd, short event, void* argv);
void sock_read_cb(int fd, short event, void* argv);

int main () {

    int sockfd = client_connect_server(SERVER_PORT);
    printf("connect to server successful\n");

    struct event_base* base = event_base_new();

    struct event* ev_sock = event_new(base, sockfd, EV_READ | EV_PERSIST, sock_read_cb, NULL);

    event_add(ev_sock, NULL);

    struct event* ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, cmd_read_cb, (void*)&sockfd);

    event_add(ev_cmd, NULL);

    event_base_dispatch(base);

    printf("finished \n");

    return 0;
}

void cmd_read_cb(int fd, short event, void* argv) {
    int n;
    int sockfd = *((int*)argv);
    char buf[BUFSIZ];
    n = read(fd, buf, sizeof(buf));
    if(n < 0) {
        perror("read cmd failed\n");
        exit(1);
    } 
    write(sockfd, buf, n);
}

void sock_read_cb(int fd, short event, void* argv) {
    int n;
    char buf[BUFSIZ];
    n = read(fd, buf, sizeof(buf));
    if(n < 0) {
        perror("read cmd failed\n");
        exit(1);
    } else if(n == 0) {
        printf("the other side has closed\n");
        close(fd);
    }
    write(STDOUT_FILENO, buf, n);
}

int client_connect_server(short server_port) {
    int sockfd, op;
    struct sockaddr_in client_addr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("socket failed\n");
        exit(1);
    }

    len = sizeof(client_addr);
    bzero(&client_addr, len);
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(server_port);

    op = connect(sockfd, (struct sockaddr*) &client_addr, len);
    if(op < 0) {
        perror("connect failed\n");
        exit(1);
    }

    return sockfd;
}