#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    // 1. 创建通信的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket error");
        exit(0);
    }

    // 2. 连接服务器
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9977); // 大端端口
    // 81.68.113.233
    inet_pton(AF_INET, "10.0.4.2", &addr.sin_addr.s_addr);

    int ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect error");
        exit(0);
    }

    // 3. 和服务器端通信
    int num = 0;
    char buf[1024] = {0};
    while (1)
    {
        // 发送数据
        sprintf(buf, "hello, world...%d\n", num++);
        // scanf("%s\n", buf);
        printf("%s\n", buf);
        write(fd, buf, strlen(buf) + 1);

        // 接收数据
        // memset(buf, 0, sizeof(buf));
        int len = read(fd, buf, sizeof(buf));
        if (len > 0)
        {
            printf("服务器: %s\n", buf);
        }
        else if (len == 0)
        {
            printf("服务器断开了连接...\n");
            break;
        }
        else
        {
            perror("read");
            break;
        }
        // sleep(1);
    }

    close(fd);

    return 0;
}
