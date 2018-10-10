#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 7777

int main(int argc, char const *argv[])
{
    int cfd;
    struct sockaddr_in server_addr;
    char buf[BUFSIZ];
    int len;

    cfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    connect(cfd, (struct sockaddr*) &server_addr, sizeof(server_addr));

    while(1) {
        fgets(buf, sizeof(buf), stdin);
        write(cfd, buf, strlen(buf));
        len = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, len);
    }

    close(cfd);

    return 0;
}
