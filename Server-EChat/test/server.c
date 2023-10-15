#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

// epoll边沿模式
// 非阻塞
// 多线程

typedef struct socketinfo
{
    int fd;
    int ep_fd;
} SocketInfo;

void *acceptConnect(void *arg)
{
    printf("acceptConnect tid:%ld\n", pthread_self());

    SocketInfo *info = (SocketInfo *)arg;
    int new_fd = accept(info->fd, NULL, 0);

    // 将新的文件描述符设置为非阻塞
    int flag = fcntl(new_fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(new_fd, F_SETFL, flag);

    // 将文件描述符添加至epoll中
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN | EPOLLET; // 设置为边缘检测
    ep_event.data.fd = new_fd;
    int ret = epoll_ctl(info->ep_fd, EPOLL_CTL_ADD, new_fd, &ep_event);
    if (-1 == ret)
    {
        perror("epoll_ctl accept error");
        exit(0);
    }
    free(info);
    return NULL;
}

void *communication(void *arg)
{
    printf("communication tid:%ld\n", pthread_self());

    SocketInfo *info = (SocketInfo *)arg;

    char buf[5];
    char tempbuf[1024];
    memset(tempbuf, 0, sizeof(buf));
    while (1)
    {
        int len = recv(info->fd, buf, sizeof(buf), 0);
        if (len > 0)
        {
            printf("客户端说：%s\n", buf);
            strncat(tempbuf + strlen(buf), buf, len);
        }
        // 非阻塞模式和阻塞模式判断方式相同都是len == 0
        else if (len == -1)
        {
            if (errno == EAGAIN)
            {
                printf("数据读取完成...\n");
                send(info->fd, tempbuf, strlen(tempbuf), 0); // 回应客户端
                break;
            }
            else
            {
                perror("recv error");
                break;
            }
        }
        else
        {
            printf("客户端已断开连接...\n");
            // 必须先将结点从红黑树上删除之后在关闭文件描述符,否则会出现关闭异常
            epoll_ctl(info->ep_fd, EPOLL_CTL_DEL, info->fd, NULL);
            close(info->fd);
            break;
        }
    }
    free(info);
    return NULL;
}

int main()
{
    // 创建监听的套接字
    int l_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == l_fd)
    {
        perror("socket error");
        exit(1);
    }

    // 初始化结构体sockaddr_in
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in)); // 按字节赋值为0
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9977);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 设置端口复用
    int opt = 1;
    setsockopt(l_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定端口
    int ret = bind(l_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if (-1 == ret)
    {
        perror("bind error");
        exit(1);
    }

    // 设置监听
    ret = listen(l_fd, 128);
    if (-1 == ret)
    {
        perror("listen error");
        exit(1);
    }

    // 创建epoll模型
    int ep_fd = epoll_create(1); // 只要大于0的数就行，无实质意义
    if (-1 == ep_fd)
    {
        perror("epoll_create error");
        exit(0);
    }

    // 向epoll实例中添加需要检测结点
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN; // 检测l_fd的读缓冲区是否有数据
    ep_event.data.fd = l_fd;   // 添加用于监听的文件描述符
    ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, l_fd, &ep_event);
    if (-1 == ret)
    {
        perror("epoll_ctl error");
        exit(0);
    }

    struct epoll_event arr_ep_event[1024];
    int arr_ep_size = sizeof(arr_ep_event) / sizeof(struct epoll_event);
    while (1)
    {
        // 调用一次就检测一次
        int num = epoll_wait(ep_fd, arr_ep_event, arr_ep_size, -1);
        int curr_fd;
        pthread_t tid; // 线程ID，用于线程分离
        for (int i = 0; i < num; i++)
        {
            curr_fd = arr_ep_event[i].data.fd;

            SocketInfo *info = (SocketInfo *)malloc(sizeof(SocketInfo));
            info->fd = curr_fd;
            info->ep_fd = ep_fd;

            // 用于监听的文件描述符
            if (curr_fd == l_fd)
            {
                pthread_create(&tid, NULL, acceptConnect, info);
                pthread_detach(tid);
            }
            else // 用于通信的文件描述符
            {
                pthread_create(&tid, NULL, communication, info);
                pthread_detach(tid);
            }
        }
    }
    return 0;
}
