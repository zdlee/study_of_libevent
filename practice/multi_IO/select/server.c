#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "wrap.h"

#define SERVER_PORT 7777
#define MAX_CON 1024

int main(int argc, char const *argv[])
{
    int listenfd, connfd, maxfd;
    struct sockaddr_in serv_addr, client_addr;
    fd_set rset, allset;
    int ready, i, maxi, len, j;
    socklen_t client_addr_len;
    int clientfd[MAX_CON];
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

    printf("waiting for connect...\n");

    FD_ZERO(&allset);
    FD_SET(listenfd, &rset);
    maxfd = listenfd;
    for(i = 0; i < MAX_CON; i++) {
        clientfd[i] = -1;   
    }

    while(1) {
        rset = allset;
        ready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(ready < 0) {
            perr_exit("select failed\n");
        } 

        if(FD_ISSET(listenfd, &rset)) {
            client_addr_len = sizeof(client_addr);
            connfd = Accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
            printf("client IP : %s\tport : %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
                ntohs(client_addr.sin_port));

            FD_SET(connfd, &allset);
            if(maxfd < connfd) {
                maxfd = connfd;
            }
            for(i = 0; i < MAX_CON; i++) {
                if(clientfd[i] == -1) {
                    clientfd[i] = connfd;
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

        for(i = 0; i <= maxi; i++) {
            if(clientfd[i] != -1) {
                if(FD_ISSET(clientfd[i], &rset)) {
                    len = Read(clientfd[i], buf, sizeof(buf));
                    if(len == 0) {
                        printf("other side has closed\n");
                        FD_CLR(clientfd[i], &allset);
                        Close(clientfd[i]);
                        clientfd[i] = -1;
                    } else {
                        Write(STDOUT_FILENO, buf, len);
                        for(j = 0; j < len; j++) {
                            buf[j] = toupper(buf[j]);
                        }
                        Write(clientfd[i],buf, len);
                    }
                }
            }
            if(--ready == 0) break;
        }
    }
    Close(listenfd);
    return 0;
}
