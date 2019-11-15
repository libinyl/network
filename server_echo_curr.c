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

// 并发 echo 服务器
// 适用于长时间服务, 每个连接内可进行多次服务

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define SERVER_PORT 1234
#define SERVER_IP "127.0.0.1"
#define LISTEN_BACKLOG 5

void catch_child(int signum)
{
    while (waitpid(-1, NULL, WNOHANG));
}

void doit(int clientfd)
{
    // 100 次 echo
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    if (read(clientfd, buff, sizeof(buff))) {
        for (int j = 0; j < 100; ++j) {
            sleep(1);
            write(clientfd, buff, strlen(buff));
        }
    }
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
    struct sigaction act;
    act.sa_handler = catch_child;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    // 用于清理子进程
    sigaction(SIGCHLD, &act, NULL);

    // 接收
    struct sockaddr_in cliAddr;
    memset(&cliAddr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addrLength = sizeof(struct sockaddr_in);

    for (;;) {
        int clientfd = accept(listenfd, (struct sockaddr *) &cliAddr, &client_addrLength);
        if (clientfd < 0) {
            if (errno == EINTR)
                continue;
            else
                handle_error("accept");
        }

        if (clientfd < 0) handle_error("main");
        char clientip[1024];
        printf("客户ip:%s,端口:%d\n",
               inet_ntop(AF_INET, &cliAddr.sin_addr.s_addr, clientip, sizeof(clientip)),
               ntohs(cliAddr.sin_port));

        if (fork() == 0) {
            close(listenfd);
            doit(clientfd);
            close(clientfd);
            exit(0);
        }
        close(clientfd);
    }
}