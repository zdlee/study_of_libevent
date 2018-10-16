#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "wrap.h"

#define SERVER_PORT 7777
#define MAX_CON 1024

int main(int argc, char const *argv[])
{
    int listenfd, connfd;
    struct sockaddr_in serv_addr, client_addr;
    int ready, i, maxi, len, j;
    socklen_t client_addr_len;
    struct pollfd clients[MAX_CON];
    char buf[BUFSIZ], client_IP[BUFSIZ];

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family= AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port= htons(SERVER_PORT);

    Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    Listen(listenfd, 1024);

    clients[0].fd = listenfd;
    clients[1].events = POLLIN;

    printf("waiting for connect...\n");

    for(i = 1; i < MAX_CON; i++) {
        clients[i].fd = -1;   
    }

    maxi = 0;

    for(;;) {
        ready = poll(clients, maxi + 1, -1);
        if(ready < 0) {
            perr_exit("poll failed\n");
        } 
        printf("PP\n");
        if(clients[0].revents & POLLIN) {
            printf("PP\n");
            client_addr_len = sizeof(client_addr);
            connfd = Accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
            printf("client IP : %s\tport : %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
                ntohs(client_addr.sin_port));

            for(i = 1; i < MAX_CON; i++) {
                if(clients[i].fd == -1) {
                    clients[i].fd = connfd;
                    clients[i].events = POLLIN;
                    if(maxi < i) {
                        maxi = i;
                    }
                    break;
                }
            }
            if(--ready == 0) {
                continue;
            }
        }

        for(i = 1; i <= maxi; i++) {
            if(clients[i].fd != -1) {
                if(clients[i].revents & POLLIN) {
                    len = Read(clients[i].fd, buf, BUFSIZ);
                    if(len == 0) {
                        printf("other side has closed\n");
                        Close(clients[i].fd);
                        clients[i].fd = -1;
                    } else {
                        Write(STDOUT_FILENO, buf, len);
                        for(j = 0; j < len; j++) {
                            buf[j] = toupper(buf[j]);
                        }
                        Write(clients[i].fd, buf, len);
                    }
                }
            }
            if(--ready == 0) break;
        }  
    }
    Close(listenfd);
    return 0;
}
