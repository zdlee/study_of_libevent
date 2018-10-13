#include "wrap.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define SERVER_PORT 7777

void do_sigchild(int num) {
    while(waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    int listenfd, connfd;
    int opt = 1, n, i;
    socklen_t client_addr_len;
    int pid;
    char buf[BUFSIZ], str[INET_ADDRSTRLEN];
    struct sigaction waitchild;

    waitchild.sa_handler = do_sigchild;
    sigemptyset(&waitchild.sa_mask);
    waitchild.sa_flags = 0;
    sigaction(SIGCHLD, &waitchild, 0);

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    //不必等待2MSL时间
    //setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    Bind(listenfd, (struct sockaddr*) &server_addr, sizeof(server_addr));

    Listen(listenfd, 1024);

    printf("waiting for connect...\n");
    while(1) {
        client_addr_len = sizeof(client_addr);
        connfd = Accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
        pid = fork();
        if(pid == 0) {
            Close(listenfd);
            while(1) {
                n = Read(connfd, buf, sizeof(buf));
                if(n == 0) {
                    printf("the other side has been closed\n");
                    break;
                }
                printf("received from %s at port %d\n",
                inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
                ntohs(client_addr.sin_port));
                Write(STDOUT_FILENO, buf, n);

                for(i = 0; i < n; i++) 
                    buf[i] = toupper(buf[i]);
                Write(connfd, buf, n);
            }
            Close(connfd);
            return 0;
        } else if(pid > 0) {
            Close(connfd);
        } else {
            perr_exit("fork error");
        }
    }

    return 0;
}
