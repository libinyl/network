#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/select.h>

// 并发 echo 服务器
// 适用于长时间服务, 每个连接内可进行多次服务

#define handle_error(msg) \
           do { printf("退出");perror(msg); exit(EXIT_FAILURE); } while (0)

#define SERVER_PORT 1234
#define SERVER_IP "127.0.0.1"
#define LISTEN_BACKLOG 5

void printCliInfo(struct sockaddr_in *cliAddr)
{
    char clientip[1024];
    printf("客户ip:%s,端口:%d\n",
           inet_ntop(AF_INET, &(cliAddr->sin_addr.s_addr), clientip, sizeof(clientip)),
           ntohs(cliAddr->sin_port));
}

int main()
{
    // 创建
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        handle_error("socket");
    }

    // 绑定
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr=inet_addr(SERVER_IP),
    };
    // 地址重用
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if ((bind(listenfd, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr_in))) == -1) {
        handle_error("bind");
    }

    // 监听
    if (listen(listenfd, LISTEN_BACKLOG) == -1) {
        handle_error(("listen"));
    }

    // 接收
    struct sockaddr_in cliAddr;
    bzero(&cliAddr, sizeof(serverAddr));
    socklen_t client_addrLength = sizeof(struct sockaddr_in);

    int maxfd = listenfd;
    int nready = 0;
    char buff[1024];
    fd_set rset, allset;    // rset :传入-传出  allset:始终维护当前全部fd (012 除外)
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            handle_error("select");
        }

        if (FD_ISSET(listenfd, &rset)) {// 新连接,维护到集合
            int connfd = accept(listenfd, (struct sockaddr *) &cliAddr, &client_addrLength);

            if (connfd < 0)
                handle_error("main");

            FD_SET(connfd, &allset);
            maxfd = maxfd < connfd ? connfd : maxfd;
            printCliInfo(&cliAddr);

            if (0 == --nready) {    // 没有其他可读 fd,无需循环
                continue;
            }
        }
        for (int i = listenfd + 1; i <= maxfd; ++i) {
            if (FD_ISSET(i, &rset)) {
                memset(buff, 0, sizeof(buff));
                int n = read(i, buff, sizeof(buff));
                printf("fd:%d,n:%d\n", i, n);
                if (n < 0) {
                    handle_error("read");
                }
                else if (n == 0) {
                    close(i);
                    FD_CLR(i, &allset);
                }
                else {
                    for (int j = 0; j < 100; ++j) {
                        sleep(1);
                        int writeres;
                        if ((writeres = write(i, buff, strlen(buff))) < 0) {
                            printf("write fail,return: %d", writeres);
                        }
                    }
                }
            }
        }
    }
}