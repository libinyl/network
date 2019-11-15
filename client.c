#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define SERVER_PORT 1234
#define SERVER_IP "127.0.0.1"
#define BUF_SIZE 1024
int main()
{
    // 创建client
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        handle_error("socket");
    }

    //设置 server
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr=inet_addr(SERVER_IP),
    };

    // 连接
    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        handle_error("connect");
    }

    // 预备
    char message[BUF_SIZE];
    memset(message, 0, sizeof(message));
    fgets(message, BUF_SIZE, stdin);

    // 请求
    write(sock, message, strlen(message) - 1);

    // 响应
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    read(sock, buff, sizeof(buff));

    // 处理
    printf("%s\n", buff);
}