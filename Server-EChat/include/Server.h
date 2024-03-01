#ifndef __SERVER__H
#define __SERVER__H

#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <cstdio>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <algorithm>
#include <signal.h>
#include <pthread.h>
#include <mutex>

#include "json/json.h"
#include "DataBase.h"

#define DEFAULT_PORT 7799 // 默认端口
#define BUFFER_SIZE 1024  // 最大缓冲区大小
#define EVENTS_SIZE 1024  // 最大事件数
#define TIMEOUT -1        // 指定最大等待时间(毫秒),-1就是无限

#define PACK_HEAD_LENGTH sizeof(int)

typedef struct FDInfo
{
    int fd;
    int ep_fd;
} FDInfo;

// extern bool AddWorkFd(const int &fd);
// extern void DeleteWorkFd(const int &fd);
// extern bool QueryWorkFd(const int &fd);

extern void AddOnlineFd(const int &usrId, const int &fd);
extern void DeleteOnlineFd(const int &fd);
extern void DeleteUser(const int &usrId);
extern int GetOnlineFd(const int &usrId);
extern bool GetOnlineState(const int &usrId);
extern void ShowAllFd();

class Server
{
public:
    Server();
    ~Server();

    void Start();

private:
    void CreateSocket(); // 创建套接字
    void CreateEpoll();  // 创建epoll池
    void HandleEvent();  // 处理事件

    static void *ThreadToConnect(void *arg);     // 处理客户端连接
    static void *ThreadToCommunicate(void *arg); // 处理客户端通信

    static int Pack(const std::string &data, char **packData);         // 封包
    static int Unpack(const int &fd, char **data);                     // 拆包
    static void HandleRequest(const int &fd, const std::string &data); // 处理请求
    static void Send(const int &fd, const std::string &json);          // 发送数据

private:
    int listenFd;                           // 用于监听的文件描述符
    int epollFd;                            // epoll实例的文件描述符
    struct epoll_event events[EVENTS_SIZE]; // 事件集合

    bool runState; // 服务器运行状态
    bool dbState;  // 数据库连接状态

    static DataBase *db; // db工具
};

#endif
