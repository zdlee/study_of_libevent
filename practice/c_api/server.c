#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "wrap.h"

#define SERVER_PORT 7777

int main(int argc, char const *argv[])
{
    int sfd, cfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char client_IP[BUFSIZ], buf[BUFSIZ];
    int len, i;

    sfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    Bind(sfd, (struct sockaddr*) &server_addr, sizeof(server_addr));

    Listen(sfd, 1024);

    printf("waiting for connect...");

    client_addr_len = sizeof(client_addr);
    cfd = Accept(sfd, (struct sockaddr*) &client_addr, &client_addr_len);

    printf("client IP : %s\tport : %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
        ntohs(client_addr.sin_port));

    while(1) {
        len = Read(cfd, buf, sizeof(buf));
        Write(STDOUT_FILENO, buf, len);
        for(i = 0; i < len; i++){
            buf[i] = toupper(buf[i]);
        }
        Write(cfd, buf, len);
    }

    Close(sfd);
    Close(cfd);

    return 0;
}
