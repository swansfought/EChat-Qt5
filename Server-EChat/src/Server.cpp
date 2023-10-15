#include "Server.h"

DataBase *Server::db = nullptr;

std::mutex g_mtx_fork; // 全局互斥锁
std::mutex g_mtx;      // 全局互斥锁

Server::Server()
{
    runState = false;
    db = DataBase::GetInstance();
}

Server::~Server()
{
    db->CloseDB();
    close(epollFd);
    close(listenFd);
}

// 启动服务器
void Server::Start()
{
    if (runState)
        return;

    CreateSocket();  // 1.初始化服务器套接字
    CreateEpoll();   // 2.初始化epoll
    runState = true; // 3.服务器运行标识
    db->ConnectDB(); // 4.连接数据库
    HandleEvent();   // 5.处理epoll事件
}

// 创建套接字
void Server::CreateSocket()
{
    // 1.创建监听的套接字
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listenFd)
    {
        perror("socket create error");
        exit(1);
    }

    // 2.初始化结构体sockaddr_in
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 3.设置端口复用
    int opt = 1;
    int ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (-1 == ret)
    {
        perror("setsockopt error");
        close(listenFd);
        exit(1);
    }

    // 4.绑定端口
    ret = bind(listenFd, (struct sockaddr *)&addr, sizeof(addr));
    if (-1 == ret)
    {
        perror("bind error");
        exit(1);
    }

    // 5.设置监听
    ret = listen(listenFd, 64);
    if (-1 == ret)
    {
        perror("listen error");
        exit(1);
    }
    cout << "--------------------------------------------\n";
    printf("[log]server start listen port[%d]...\n", DEFAULT_PORT);
}

// 创建epoll
void Server::CreateEpoll()
{
    // 创建epoll模型
    epollFd = epoll_create(1);
    if (-1 == epollFd)
    {
        perror("epoll_create error");
        exit(0);
    }

    // 添加fd到epoll中
    struct epoll_event ev;
    ev.events = EPOLLIN;   // 检测listenFd的读缓冲区是否有数据
    ev.data.fd = listenFd; // 添加用于监听的文件描述符
    int ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &ev);
    if (-1 == ret)
    {
        perror("epoll_ctl error");
        exit(0);
    }
    cout << "[log]server epoll create successfully...\n";
}

// 处理事件
void Server::HandleEvent()
{
    cout << "[log]server start handle event...\n";
    cout << "--------------------------------------------\n\n";
    int lastFd = 0; // 用来记录上次的文件描述符
    while (1)
    {
        // 调用一次就检测一次
        int num = epoll_wait(epollFd, events, EVENTS_SIZE, TIMEOUT);
        if (-1 == num)
        {
            perror("epoll_wait error");
            exit(0);
        }
        pthread_t tid; // 线程ID，用于线程分离
        for (int i = 0; i < num; i++)
        {
            int curr_fd = events[i].data.fd;

            FDInfo *info = (FDInfo *)malloc(sizeof(FDInfo));
            info->fd = curr_fd;
            info->ep_fd = epollFd;

            if (curr_fd == listenFd)
            {
                pthread_create(&tid, nullptr, ThreadToConnect, info);
                pthread_detach(tid);
            }
            else
            {

                pthread_create(&tid, nullptr, ThreadToCommunicate, info);
                pthread_detach(tid);
                // std::lock_guard<std::mutex> guard(g_mtx_fork);
                // if (!QueryWorkFd(curr_fd))
                // {
                //     AddWorkFd(curr_fd);
                //     pthread_create(&tid, nullptr, ThreadToCommunicate, info);
                //     pthread_detach(tid);
                // }
            }
        }
    }
}

// 与客户端建立连接
void *Server::ThreadToConnect(void *arg)
{
    // 频繁的线程创建和销毁！
    // printf("tid[%ld] connecting ... \n", pthread_self());

    // 1.获取fd
    FDInfo *info = (FDInfo *)arg;
    int fd = info->fd;
    int ep_fd = info->ep_fd;

    // 2.获取新连接的fd
    int new_fd = accept(fd, nullptr, nullptr);

    // 3.将fd设置为非阻塞
    int flag = fcntl(new_fd, F_GETFL);
    fcntl(new_fd, F_SETFL, flag |= O_NONBLOCK);

    // 4.将fd添加至epoll池中
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLET; // 设置为边缘检测
    ev.data.fd = new_fd;
    int ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, new_fd, &ev);
    if (-1 == ret)
    {
        perror("connect epoll_ctl error");
        exit(0);
    }

    // tip
    // printf("tid[%ld] successfully connected... \n", pthread_self());

    free(info);
    info = nullptr;
    return nullptr;
}

// 与客户端通信
void *Server::ThreadToCommunicate(void *arg)
{
    // printf("tid[%ld] communicating...\n", pthread_self());

    // 获取fd
    FDInfo *info = (FDInfo *)arg;
    int fd = info->fd;
    int ep_fd = info->ep_fd;

    // 接收数据
    char *data = nullptr;
    while (true)
    {
        int ret = Unpack(fd, &data); // 拆包
        if (0 == ret)
        {
            DeleteOnlineFd(fd); // 用户下线
            // db->DeleteOnlineUser(usrId); // 直接从数据库删除
            // DeleteWorkFd(fd); // 此文件描述符已完成工作

            cout << "客户端已断开连接...\n";
            ret = epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, nullptr);
            if (-1 == ret)
            {
                // 释放资源
                free(data);
                data = nullptr;

                perror("communicate epoll_ctl error");
                exit(0);
            }
            close(fd); // 关闭文件描述符
            break;
        }
        else if (-1 == ret)
        {
            break;
        }
        else
            HandleRequest(fd, data);

        // 释放资源，进入下一轮读取
        free(data);
        data = nullptr;
        // usleep(200);
    }

    // 释放资源，结束通信
    free(info);
    info = nullptr;
    free(data);
    data = nullptr;
    // DeleteWorkFd(fd); // 此文件描述符已完成工作

    return nullptr;
}

/**
 * @brief 封包
 * @param {string} &json 待封装数据
 * @param {char} ** 保存接收的数据
 * @return {bool} 返回总字节数
 */
int Server::Pack(const std::string &data, char **packData)
{
    int dataLen = data.length();
    if (dataLen <= 0)
        return 0;

    // 封装数据
    int ndataLen = htonl(dataLen);             // 网络字节序
    int totalLen = PACK_HEAD_LENGTH + dataLen; // 总长度：包头长度 + 数据长度
    *packData = new char[totalLen];            // (char *)malloc(totalLen)

    memcpy(*packData, &ndataLen, PACK_HEAD_LENGTH);
    memcpy(*packData + PACK_HEAD_LENGTH, data.c_str(), dataLen);

    // cout << "totalLen=" << totalLen << endl;

    return totalLen;
}

/**
 * @brief 拆包获取数据，无数据时等待
 * @param {int} &fd
 * @param {char} ** 保存接收的数据
 * @return {int} 返回已接收字节, 0  -1  已接收字节数
 */
int Server::Unpack(const int &fd, char **data)
{
    std::lock_guard<std::mutex> guard(g_mtx);

    int dataLen = 0; // 数据长度
    int recvLen = 0; // 已接收长度
    int retLen = 0;  // 接收返回值

    // 拆包头
    char head[4];
    while (recvLen < PACK_HEAD_LENGTH)
    {
        retLen = read(fd, head + recvLen, PACK_HEAD_LENGTH - recvLen); // 阻塞读数据
        if (0 == retLen)
            return 0;
        else if (-1 == retLen)
        {
            // 客户端断开连接
            if (errno == ECONNRESET) // EAGAIN
                return 0;
            // 数据读取完成
            if (errno != EAGAIN)
                perror("unpack head recv");
            return -1;
            // cout << "recv: " << strerror(errno) << endl;
        }
        recvLen += retLen;
        // cout << "Unpack recvLen=" << recvLen << endl;
    }
    recvLen = 0;
    retLen = 0;
    dataLen = ntohl(*((int *)head)); // 转换成主机字节序

    // 读数据
    // cout << "Unpack dataLen=" << dataLen << endl;
    // *data = new char[dataLen];
    *data = (char *)malloc(dataLen * sizeof(char)); // 分配内存
    if (*data == NULL)
    {
        cout << "*data 内存分配错误！试图为其分配" << dataLen << "内存大小..." << endl;
        return -1;
    }

    while (recvLen < dataLen)
    {
        retLen = read(fd, *data + recvLen, dataLen - recvLen);
        if (0 == retLen)
            return 0;
        else if (-1 == retLen)
        {
            if (errno == ECONNRESET) // EAGAIN
                return 0;
            if (errno == EAGAIN)
            {
                usleep(100 * 1000); // 适当延时 100ms
                continue;
            }
            perror("unpack data recv");
            return -1;
        }
        recvLen += retLen;
    }
    // cout << "Unpack dataLen=" << dataLen << endl;
    // cout << "Unpack recvLen = " << recvLen << endl;

    return recvLen;
}

// 发送数据
void Server::Send(const int &fd, const std::string &json)
{
    cout << "send: " << json << endl;

    char *packJson;
    int totalLen = Pack(json, &packJson);
    if (totalLen <= 0)
    {
        cout << "Send：试图发送空数据?" << endl;
        return;
    }

    // 发送数据
    int writeLen = 0;
    int retLen = 0;
    while (writeLen < totalLen)
    {
        retLen = write(fd, packJson + writeLen, totalLen - writeLen);
        if (0 == retLen)
        {
            usleep(100 * 1000); // 适当延时 100ms
            continue;
        }
        else if (-1 == retLen)
        {
            perror("Send write error：");
            return;
        }
        writeLen += retLen;
        usleep(100 * 1000); // 适当延时
    }
    // cout << "Send totalLen=" << totalLen << endl;
    // cout << "Send writeLen = " << writeLen << endl;
    //  << endl;

    // 释放资源
    delete[] packJson;
    packJson = nullptr;

    usleep(200 * 1000); // 适当延时 200ms
}

// 处理请求
void Server::HandleRequest(const int &fd, const std::string &data)
{
    cout << "recv:" << data << endl;

    // 1.获取客户请求信息
    Json::Reader reader;
    Json::Value rootObj;
    reader.parse(data, rootObj);
    int code = rootObj["code"].asInt();
    int usrId;
    if (503 != code || 104 != code || 103 != code)
        usrId = rootObj["id"].asInt();

    // 2.服务器回应请求处理
    Json::Value _rootObj;
    Json::Value outValue; // 用于接收查询返回值
    _rootObj["code"] = code;
    _rootObj["state"] = false;
    _rootObj["id"] = usrId;

    switch (code)
    {
    case 77: // 用户建议
    {
        std::string text = rootObj["recommend"].asString();
        if (db->AddUserRecommends(usrId, text))
            _rootObj["state"] = true;

        break;
    }
    case 88: // 换账号问题，需要删除文件描述符对应的用户id
    {
        // ShowAllFd();
        DeleteUser(rootObj["lastId"].asInt());
        _rootObj["state"] = true;
        // DeleteWorkFd(fd); // 此文件描述符已完成工作
        // ShowAllFd();
        break;
    }
    case 99: // 成功连接
    {
        AddOnlineFd(usrId, fd); // 用户上线
        // cout << "userId=" << usrId << " fd=" << fd << endl;

        _rootObj.removeMember("id");
        _rootObj["state"] = true;
        break;
    }
    case 101: // 验证用户✔
    {
        std::string userPwd = rootObj["password"].asString();
        // if (db->Login(usrId, userPwd))
        // {
        //     // 用户已在线情况，此时无法登录
        //     if (GetOnlineFd(usrId) > 0)
        //     {
        //         _rootObj["loginState"] = true;
        //     }
        //     else
        //     {
        //         AddOnlineFd(usrId, fd); // 用户上线
        //         // cout << "userId=" << usrId << " fd=" << GetOnlineFd(usrId) << endl;
        //         if (db->QueryUserInfo(outValue, usrId))
        //         {
        //             _rootObj["nickname"] = outValue["nickname"];
        //             _rootObj["picture"] = outValue["picture"];
        //             _rootObj["loginState"] = false;
        //         }
        //     }
        //     _rootObj["state"] = true;
        // }
        AddOnlineFd(usrId, fd); // 用户上线
        // cout << "userId=" << usrId << " fd=" << GetOnlineFd(usrId) << endl;
        if (db->QueryUserInfo(outValue, usrId))
        {
            _rootObj["nickname"] = outValue["nickname"];
            _rootObj["picture"] = outValue["picture"];
            _rootObj["loginState"] = false;
        }
        _rootObj["state"] = true;
        break;
    }
    case 102: // 注册用户✔
    {
        std::string nickname = rootObj["nickname"].asString();
        std::string userPwd = rootObj["password"].asString();

        int newId;
        if (!db->getNewID(newId))
            break;

        if (db->Register(newId, nickname, userPwd))
        {
            _rootObj["state"] = true;
            _rootObj["newId"] = newId;
            db->DeleteNewID(newId);
        }
        break;
    }
    case 103: // 发送文件消息
    {
        rootObj.removeMember("id");
        int receiver = rootObj["receiver"].asInt();
        bool sendState = rootObj["sendState"].asBool();
        // std::string fineName = "/home/EChat_Data/" + rootObj["fileName"].asString(); // 获取文件名称

        int fd = GetOnlineFd(receiver);
        // cout << fd << endl;
        // return;
        if (fd > 0)
        { // 发送中
            Send(fd, data);
        }
        else
        {
            if (!sendState)
            { // 数据没发完就断网了，或者对方不在线
                _rootObj["state"] = false;
                _rootObj["id"] = usrId;
                _rootObj["receiver"] = rootObj["receiver"].asInt();
                _rootObj["sendState"] = rootObj["sendState"].asBool();
                Json::FastWriter writer;
                std::string json = writer.write(rootObj);
                Send(fd, json);
            }
        }

        return; // 直接返回
    }
    case 104: // 客户端接收文件消息回应，在这删除服务器缓存
    {
        break;
    }
    case 501: // 增好友✔
    {
        int applicant = rootObj["applicant"].asInt();
        bool agree = rootObj["agree"].asBool();
        _rootObj["agree"] = agree;
        _rootObj["applicant"] = applicant;
        // 同意
        if (agree)
        {
            Json::Value replyJson;

            if (!db->AddFriend(applicant, usrId))
                return;

            // 申请人
            replyJson["identify"] = 0;
            replyJson["receiver"] = usrId;
            replyJson["code"] = 903;
            replyJson["state"] = true;
            replyJson["type"] = 0;
            replyJson["receiverInfo"];
            if (db->QueryUserInfo(outValue, usrId))
            {
                outValue.removeMember("email");
                outValue.removeMember("phone");
                outValue.removeMember("region");
                outValue.removeMember("self_say");
                outValue.removeMember("sex");
                outValue.removeMember("userId");
                replyJson["receiverInfo"] = outValue;
            }
            int fd = GetOnlineFd(applicant);
            if (fd > 0)
            {
                Json::FastWriter writer;
                std::string json = writer.write(replyJson);
                Send(fd, json);
            }

            // 处理者
            outValue.clear();
            replyJson.clear();
            replyJson["identify"] = 1;
            replyJson["applicant"] = applicant;
            replyJson["code"] = 903;
            replyJson["state"] = true;
            replyJson["type"] = 0;
            replyJson["applicantInfo"];
            if (db->QueryUserInfo(outValue, applicant))
            {
                outValue.removeMember("email");
                outValue.removeMember("phone");
                outValue.removeMember("region");
                outValue.removeMember("self_say");
                outValue.removeMember("sex");
                outValue.removeMember("userId");
                replyJson["applicantInfo"] = outValue;
            }
            fd = GetOnlineFd(usrId);
            if (fd > 0)
            {
                Json::FastWriter writer;
                std::string json = writer.write(replyJson);
                Send(fd, json);
            }

            db->DeleteUserApply(applicant, usrId, 0); // 删除好友申请
            _rootObj["state"] = true;
        }
        else
        {
            db->DeleteUserApply(applicant, usrId, 0); // 删除好友申请
            _rootObj["state"] = true;
        }
        break;
    }
    case 502: // 增群成员(进群)✔
    {
        int applicant = rootObj["applicant"].asInt();
        bool agree = rootObj["agree"].asBool();
        int grpId = rootObj["groupId"].asInt();

        // 先给处理反馈
        _rootObj["agree"] = agree;
        _rootObj["groupId"] = grpId;
        _rootObj["state"] = true;
        Json::FastWriter writer;
        Send(fd, writer.write(_rootObj));

        // 同意
        if (agree)
        {

            // 拿到原来的群成员
            if (!db->QueryGroupMember(outValue, grpId))
                break;
            outValue["members"].append(applicant); // 增加群成员
            // 没做判断!!!!!!!
            bool ret = db->UpdateGroupMember(applicant, grpId, writer.write(outValue["members"]));
            bool _ret = db->AddUserInGroup(applicant, grpId, "群成员");

            outValue.clear();
            Json::Value replyJson;
            replyJson["groupInfo"];
            replyJson["code"] = 903;
            replyJson["state"] = true;
            replyJson["type"] = 1;
            replyJson["groupId"] = grpId;
            if (db->QueryGroupInfo(outValue, grpId))
            {
                Json::Value subObj;
                subObj["nickname"] = outValue["nickname"];
                subObj["picture"] = outValue["picture"];
                replyJson["groupInfo"] = subObj;
            }
            int fd = GetOnlineFd(applicant);
            if (fd > 0)
            {
                Json::FastWriter writer;
                std::string json = writer.write(replyJson);
                Send(fd, json);
            }

            db->DeleteUserApply(applicant, usrId, 1); // 删除群聊申请
        }
        else
            db->DeleteUserApply(applicant, usrId, 1); // 删除群聊申请

        return;
    }
    case 503: // 发送信息--------------------------|
    {
        int sender = rootObj["sender"].asInt();
        int receiver = rootObj["receiver"].asInt();
        int type = rootObj["type"].asInt();

        // 1.添加到聊天对象，避免用户移除聊天对象收不到信息
        db->AddChatObject(receiver, sender, type);

        // 2.拿到发送者头像姓名等
        rootObj["senderInfos"];
        if (db->QueryUserInfo(outValue, sender))
        {
            outValue.removeMember("email");
            outValue.removeMember("phone");
            outValue.removeMember("region");
            outValue.removeMember("self_say");
            outValue.removeMember("sex");
            outValue.removeMember("userId");
        }
        // 3.如果是群聊，用用户群昵称替代原昵称
        if (type)
        {
            std::string userGroupNickname;
            // 查询用户群昵称
            if (db->QueryUserGroupNickname(sender, receiver, userGroupNickname))
                outValue["nickname"] = userGroupNickname;
        }
        rootObj["senderInfos"] = outValue;
        rootObj.removeMember("id");
        rootObj["state"] = true;
        Json::FastWriter writer;
        std::string json = writer.write(rootObj);
        _rootObj.removeMember("id");

        _rootObj["code"] = 504;    // 503发送消息回应
        _rootObj["receiverState"]; // 接收状态 离线

        if (!type)
        { // 好友
            int frdId = rootObj["receiver"].asInt();
            int frdFd = GetOnlineFd(frdId); // 在线用户fd

            // 用户在线，直接发，客户端那边自行处理没有改聊天对象的情况
            if (frdFd > 0)
            {
                Send(frdFd, json);

                _rootObj["receiverState"] = true; // 在线
                _rootObj["state"] = true;
            }
            else
            {
                // 暂存到到离线表中
                if (db->AddOfflineMsg(type, rootObj))
                {
                    _rootObj["receiverState"] = false; // 接收状态 离线
                    _rootObj["state"] = true;
                    db->AddChatObject(frdFd, sender, 0); // 添加聊天对象
                }
            }
        }
        else
        { // 群聊
            int grpId = rootObj["receiver"].asInt();

            // 迭代群map拿到群成员，在线直接发，不在线发离线消息
            auto it = groups.find(grpId);
            if (groups.end() == it)
                break;

            // 给群成员发信息
            auto members = it->second;
            int memCount = members.size();
            for (int i = 0; i < memCount; ++i)
            {
                int memberId = members[i];
                // 不给自己发,自己可能是群主/群成员
                if (memberId == sender)
                    continue;
                // cout << memberId << endl;

                int _frdFd = GetOnlineFd(memberId);
                // 在线
                if (_frdFd > 0)
                    Send(_frdFd, json);
                else
                {
                    db->AddOfflineMsg(type, rootObj, memberId);
                    db->AddChatObject(memberId, grpId, 1); // 添加聊天对象
                }
            }
            _rootObj["state"] = true;
        }
        // break;
        return;
    }
    case 504: // 接收信息--------------------------|
    {
        // fd, db->AddChatFile(usrId);
        break;
    }
    case 505: // 增聊天对象✔
    {
        int sideId = rootObj["sideId"].asInt();
        int type = rootObj["type"].asInt();
        if (type)
        {
            if (db->AddChatObject(usrId, sideId, 1))
                _rootObj["state"] = true;
        }
        else
        {
            if (db->AddChatObject(usrId, sideId, 0))
                _rootObj["state"] = true;
        }
        break;
    }
    case 506: // 增群✔
    {
        int newId;
        if (!db->getNewID(newId))
            break;

        // 创建群
        int leaderId = rootObj["leaderId"].asInt();
        int memberCount = rootObj["memberCount"].asInt();
        int picture = rootObj["picture"].asInt();
        std::string groupName = rootObj["groupName"].asString();
        Json::FastWriter writer;
        std::string members = writer.write(rootObj["members"]);
        db->AddGroup(leaderId, newId, picture, groupName, members, 1, memberCount); // 添加到群表
        db->AddUserInGroup(leaderId, newId, "群主");                                // 添加群主到用户进群表

        // 816先去更新本地数据
        Json::Value updateObj;
        updateObj["code"] = 816;
        updateObj["state"] = true;
        updateObj["friendList"];
        updateObj["groupList"];
        outValue.clear();
        if (db->QueryGroupList(outValue, usrId))
            updateObj["groupList"] = outValue;

        Send(fd, writer.write(updateObj));
        outValue.clear();

        // 添加群主到用户进群表
        Json::Value memArr = rootObj["members"];
        int counts = memArr.size();
        if (counts > 0)
        {
            for (int i = 0; i < counts; i++)
                db->AddUserInGroup(memArr[i].asInt(), newId, "群成员");

            _rootObj["state"] = true;
            _rootObj["groupId"] = newId;

            // 发送给群创建者
            std::string json = writer.write(_rootObj);
            int fd = GetOnlineFd(usrId);
            if (fd > 0)
                Send(fd, json);

            // 发送给群成员，让群成员添加该群
            for (int i = 0; i < counts; i++)
            {
                Json::Value replyJson;
                replyJson["groupInfo"];
                replyJson["code"] = 903;
                replyJson["state"] = true;
                replyJson["type"] = 1;
                replyJson["groupId"] = newId;
                Json::Value subObj;
                subObj["nickname"] = groupName;
                subObj["picture"] = picture;
                replyJson["groupInfo"] = subObj;

                // 发送给群成员
                std::string json = writer.write(replyJson);
                int fd = GetOnlineFd(memArr[i].asInt());
                if (fd > 0)
                    Send(fd, json);
            }
        }

        return;
    }
    case 601: // 删好友✔
    {
        int frdId = rootObj["friendId"].asInt();
        if (db->DeleteFriend(usrId, frdId))
        {
            _rootObj["state"] = true;

            // 删除离线消息
            db->DeleteOfflineMsg(usrId);
            db->DeleteOfflineMsg(frdId);

            // 自己把好友删除
            _rootObj["friendId"] = frdId;
            Json::FastWriter writer;
            std::string json = writer.write(_rootObj);
            Send(fd, json);

            // 通知好友把自己删除
            _rootObj["friendId"] = usrId;
            json = writer.write(_rootObj);
            int frdFd = GetOnlineFd(frdId);
            if (frdFd > 0)
                Send(fd, json);

            return; // 直接返回，不需要再次发送
        }
        else
            _rootObj["state"] = false;

        break;
    }
    case 602: // 退群(删群成员)✔
    {
        int grpId = rootObj["groupId"].asInt();
        _rootObj["state"] = false;

        // 1.拿到群成员
        if (!db->QueryGroupMember(outValue, grpId))
            break;
        if (outValue["members"].isNull())
            break;

        // 2.处理新的群成员数组
        Json::Value memArr = outValue["members"];
        Json::Value newArr;
        int counts = memArr.size();
        for (int i = 0; i < counts; i++)
        {
            // 忽略自己就是把自己删除
            if (usrId == memArr[i].asInt())
                continue;
            newArr.append(memArr[i].asInt());
        }

        // 3.保存数据
        Json::FastWriter writer;
        bool ret = db->UpdateGroupMember(usrId, grpId, writer.write(newArr)); // 更新群表
        bool _ret = db->DeleteUserInGroup(usrId, grpId);                      // 更新用户进群表
        if (ret == _ret)
        {
            _rootObj["state"] = true;
            _rootObj["groupId"] = grpId;
        }

        break;
    }
    case 603: // 删群，删除用户进去表数据、群表数据、群聊天✔
    {
        int grpId = rootObj["groupId"].asInt();

        // 1.发送给群主，去删除该群
        Json::Value reply;
        _rootObj["groupId"] = grpId;
        _rootObj["state"] = true;
        Json::FastWriter writer;
        Send(fd, writer.write(_rootObj));

        // 2.删除群主的相关信息
        db->DeleteUserInGroup(usrId, grpId); // 群主用户进群表
        db->DeleteOfflineMsg(usrId);         // 群主离线消息表

        // 3.获取群成员
        Json::Value memArr;
        if (db->QueryGroupMember(outValue, grpId))
            memArr = outValue["members"];

        // 4.删除群成员的相关信息
        int counts = memArr.size();
        for (int i = 0; i < counts; i++)
        {
            int memId = memArr[i].asInt();

            // 发送给群成员，去删除该群
            Json::Value reply;
            _rootObj["groupId"] = grpId;
            _rootObj["state"] = true;
            int fd = GetOnlineFd(memId);
            if (fd > 0)
                Send(fd, writer.write(_rootObj));

            db->DeleteUserInGroup(memId, grpId);
            db->DeleteOfflineMsg(memId);
        }

        // 4.最后删群
        db->DeleteGroup(grpId);

        return; // 直接返回
    }
    case 604: // 删聊天文件
    {

        break;
    }
    case 605: // 删聊天对象✔
    {
        int sideId = rootObj["sideId"].asInt(); // 对方id
        int type = rootObj["type"].asInt();     // 0-好友  1-群聊
        if (type)
        {
            db->DeleteChatObject(usrId, sideId, 1);
            _rootObj["state"] = true;
        }
        else
        {
            db->DeleteChatObject(usrId, sideId, 0);
            _rootObj["state"] = true;
        }
        break;
    }
    case 606: // 删除不在需要保留的离线消息
    {
        if (db->DeleteOfflineMsg(usrId))
            _rootObj["state"] = true;
        break;
    }
    case 701: // 改用户信息
    {
        if (db->UpdateUserInfo(usrId, rootObj))
            _rootObj["state"] = true;
        break;
    }
    case 702: // 改用户配置
    {
        // db->UpdateConfig(usrId);
        break;
    }
    case 703: // 修改群昵称 ✔
    {
        int grpId = rootObj["groupId"].asInt();
        int type = rootObj["type"].asInt();
        _rootObj["type"] = type;
        if (type)
        {
            // 修改群信息
            if (db->UpdateGroupInfo(grpId, rootObj["nickname"].asString(), rootObj["intro"].asString()))
            {
                _rootObj["state"] = true;
                // _rootObj["nickname"] = rootObj["nickname"].asString();
                // _rootObj["intro"] = rootObj["intro"].asString();
            }
        }
        else
        {
            // 修改群用户群昵称
            if (db->UpdateUserGroupNickname(usrId, grpId, rootObj["userGroupNickname"].asString()))
            {
                _rootObj["state"] = true;
                _rootObj["userGroupNickname"] = rootObj["userGroupNickname"];
            }
        }
        break;
    }
    case 704: // 修改密码
    case 705: // 重置密码
    {
        if (db->UpdateUserPassword(usrId, rootObj["password"].asString()))
            _rootObj["state"] = true;
        break;
    }
    case 706: // 修改头像
    {
        int picture = rootObj["picture"].asInt();
        int type = rootObj["type"].asInt();
        int id = 0;
        if (type)
        {
            id = rootObj["groupId"].asInt();
            if (db->UpdateGroupPicture(id, picture))
            {
                _rootObj["state"] = true;
            }
        }
        else
        {
            id = usrId;
            if (db->UpdateUserPicture(id, picture))
            {
                _rootObj["state"] = true;
            }
        }
        _rootObj["picture"] = picture;
        _rootObj["id"] = id;

        break;
    }
    case 801: // 查用户信息 ✔
    {
        _rootObj["userInfos"]; // 待添加的数据
        if (db->QueryUserInfo(outValue, usrId))
        {
            _rootObj["state"] = true;
            _rootObj["userInfos"] = outValue;
        }
        break;
    }
    case 802: // 查群列表✔
    {
        _rootObj["groupList"]; // 待添加的数据
        if (db->QueryGroupList(outValue, usrId))
        {
            _rootObj["groupList"] = outValue;
        }
        _rootObj["state"] = true;
        break;
    }
    case 803: // 查群信息✔
    {
        int grpId = rootObj["groupId"].asInt();
        // 1.获取群信息
        if (db->QueryGroupInfo(outValue, grpId))
        {
            _rootObj["groupInfos"] = outValue;
            outValue.clear();
        }
        _rootObj["userPicture"] = outValue["picture"];

        // 2.当前用户在群里的信息
        _rootObj["userGroupNickname"];
        if (db->QueryGroupRelation(outValue, usrId, grpId))
        {
            std::string nickname = outValue["userGroupNickname"].asString();

            outValue.clear();
            // 获取用户群昵称
            if (db->QueryUserInfo(outValue, usrId))
            {
                if (nickname == "")
                    _rootObj["userGroupNickname"] = outValue["nickname"];
                else
                    _rootObj["userGroupNickname"] = nickname;
            }
            outValue.clear();
        }

        // 3.获取群成员
        _rootObj["members"];
        if (db->QueryGroupMember(outValue, grpId))
        {
            Json::Value memberArr = outValue["members"];
            outValue.clear();
            for (int i = 0; i < memberArr.size(); ++i)
            {
                Json::Value menberInfo;
                if (db->QueryUserInfo(outValue, memberArr[i].asInt()))
                {
                    menberInfo["userId"] = memberArr[i].asInt();
                    menberInfo["nickname"] = outValue["nickname"];
                    menberInfo["picture"] = outValue["picture"];
                    _rootObj["members"].append(menberInfo);
                }
            }
            _rootObj["state"] = true;
        }

        break;
    }
    case 804: // 查询群(用于添加群聊)✔
    {
        int grpId = rootObj["groupId"].asInt();
        if (!db->QueryGroup(grpId))
            break;

        _rootObj["added"] = false;
        _rootObj["applyTime"];
        _rootObj["buildTime"];
        if (db->QueryGroupInfo(outValue, grpId))
        {
            _rootObj["state"] = true;
            outValue.removeMember("leader");
            outValue.removeMember("members");
            outValue.removeMember("intro");
            _rootObj["groupInfos"] = outValue;
            outValue.clear();
        }

        // 等待同意
        if (db->QueryUserApply(outValue, usrId, grpId, 1))
        {
            _rootObj["added"] = true;
            _rootObj["applyTime"] = outValue["applyTime"];
            _rootObj["ps"] = outValue["ps"];
        }
        else if (db->QueryGroupRelation(outValue, usrId, grpId))
        {
            _rootObj["added"] = true;
            _rootObj["joinTime"] = outValue["joinTime"];
        }
        break;
    }
    case 805: // 查群成员 ✔
    {
        int grpId = rootObj["groupId"].asInt();
        _rootObj["members"];
        if (db->QueryGroupMember(outValue, grpId))
        {
            Json::Value memberArr = outValue["members"];
            outValue.clear();
            for (int i = 0; i < memberArr.size(); ++i)
            {
                Json::Value menberInfo;
                if (db->QueryUserInfo(outValue, memberArr[i].asInt()))
                {
                    menberInfo["userId"] = memberArr[i].asInt();
                    menberInfo["nickname"] = outValue["nickname"];
                    menberInfo["picture"] = outValue["picture"];
                    _rootObj["members"].append(menberInfo);
                }
            }
            _rootObj["state"] = true;
        }
        break;
    }
    case 806: // 查好友列表(建群) ✔
    case 807: // 查好友列表(初始化)-包括自己 ✔
    {
        _rootObj["selfInfos"];
        if (db->QueryUserInfo(outValue, usrId))
        {
            _rootObj["state"] = true;
            outValue.removeMember("email");
            outValue.removeMember("phone");
            outValue.removeMember("region");
            outValue.removeMember("self_say");
            outValue.removeMember("sex");
            outValue.removeMember("userId");
            _rootObj["selfInfos"] = outValue;
        }
        outValue.clear();

        _rootObj["friendList"];
        if (db->QueryFriendList(outValue, usrId))
        {
            _rootObj["state"] = true;
            _rootObj["friendList"] = outValue;
        }
        break;
    }
    case 808: // 查询好友(用于添加好友) ✔
    {
        int frdId = rootObj["friendId"].asInt();
        if (!db->QueryUser(frdId))
            break;

        _rootObj["added"] = false;
        _rootObj["applyTime"];
        _rootObj["buildTime"];
        if (db->QueryUserInfo(outValue, frdId))
        {
            _rootObj["state"] = true;
            outValue.removeMember("sex");
            outValue.removeMember("email");
            outValue.removeMember("phone");
            outValue.removeMember("region");
            outValue.removeMember("self_say");
            _rootObj["userInfos"] = outValue;
            outValue.clear();
        }
        if (usrId != frdId)
        {
            // 等待同意
            if (db->QueryUserApply(outValue, usrId, frdId, 0))
            {
                _rootObj["added"] = true;
                _rootObj["applyTime"] = outValue["applyTime"];
                _rootObj["ps"] = outValue["ps"];
                break;
            }
            std::string buildTime;
            if (db->QueryFriendRelation(buildTime, usrId, frdId))
            {
                _rootObj["buildTime"] = buildTime;
                _rootObj["added"] = true;
            }
        }
        else
            _rootObj["added"] = true; // 试图添加自己
        break;
    }
    case 809: // 查好友详细信息(展示) ✔
    {
        _rootObj["friendInfos"]; // 待添加的数据
        int frdId = rootObj["friendId"].asInt();
        if (db->QueryFriendInfo(outValue, usrId, frdId))
        {
            _rootObj["friendInfos"] = outValue;
            _rootObj["state"] = true;
        }
        break;
    }
    case 810: // 查-810：(UI控件初始化)
    {
        if (db->InitUI(outValue, usrId))
        {
            _rootObj["chatObjectList"] = outValue["chatObjectList"];
            _rootObj["friendList"] = outValue["friendList"];
            _rootObj["groupList"] = outValue["groupList"];
            _rootObj["state"] = true;
        }
        break;
    }
    case 811: // 查聊天对象(UI初始化) ✔
    {
        _rootObj["chatObjectList"]; // 待添加的数据
        if (db->QueryChatObjectList(outValue, usrId))
        {
            _rootObj["chatObjectList"] = outValue;
            _rootObj["state"] = true;
        }
        break;
    }
    case 812: // 询聊天记录(用于初始化) -- 独立变量数据
    {
        if (db->QueryChatObjectIDList(outValue, usrId))
        {
            if (outValue.isNull())
                break;

            _rootObj["chatMsgList"]; // 回应数据
            Json::Value chatObjMsgArr;
            int counts = outValue.size();
            for (int i = 0; i < counts; i++)
            {
                Json::Value chatObjMsg;
                Json::Value chatMsg;
                int chatId = outValue[i].asInt(); // 拿到聊天对象ID
                if (db->QueryUser(chatId))
                { // 好友对象，群聊不在用户表中
                    if (db->QueryOfflineMsg(chatMsg, chatId, usrId, 0, 0))
                    {
                        chatObjMsg["chatMsgList"] = chatMsg;
                        int chatMsgCount = chatMsg.size();

                        // 获取最近一条的聊天消息
                        Json::Value lastObj = chatMsg[chatMsgCount - 1];
                        chatObjMsg["recentMsg"] = lastObj["message"];

                        chatObjMsg["unread"] = chatMsgCount;
                        chatObjMsg["type"] = 0;
                        chatObjMsg["friendId"] = chatId;
                        cout << chatId << endl;
                        // 保存好友必要信息
                        Json::Value subObj;
                        Json::Value friendInfo;
                        if (db->QueryUserInfo(subObj, chatId))
                        {
                            // chatObjMsg["nickname"] = subObj["nickname"];
                            chatObjMsg["picture"] = subObj["picture"];
                        }
                        chatObjMsgArr.append(chatObjMsg);
                    }
                }
                else if (db->QueryGroup(chatId))
                { // 群聊对象
                    if (db->QueryOfflineMsg(chatMsg, 0, usrId, chatId, 0))
                    {
                        chatObjMsg["chatMsgList"] = chatMsg;
                        int chatMsgCount = chatMsg.size();
                        chatObjMsg["unread"] = chatMsgCount; // 未读消息

                        // 获取最近一条的聊天消息
                        Json::Value lastObj = chatMsg[chatMsgCount - 1];
                        chatObjMsg["recentMsg"] = lastObj["message"];
                        chatObjMsg["type"] = 1;

                        // 保存好友必要
                        chatObjMsg["groupId"] = chatId;
                        chatObjMsgArr.append(chatObjMsg);
                    }
                }
            }
            _rootObj["state"] = true;
            _rootObj["chatMsgList"] = chatObjMsgArr;
        }
        break;
    }
    case 813: // 查聊天文件
    {
        break;
    }
    case 814: // 查用户配置(UI初始化)
    {
        if (db->QueryConfig(outValue, usrId))
        {
            _rootObj["state"] = true;
            _rootObj["configList"] = outValue;
        }
        break;
    }
    case 815: // 查询聊天对象在线状态
    {
        if (db->QueryChatObjectIDList(outValue, usrId))
        {
            if (outValue.isNull())
                break;
            _rootObj["onlineStateList"]; // 回应数据
            Json::Value singleObj;
            int counts = outValue.size();
            for (int i = 0; i < counts; i++)
            {
                int chatId = outValue[i].asInt(); // 拿到聊天对象ID
                // 群聊不在用户表中！！！
                if (!db->QueryUser(chatId))
                    continue;

                singleObj["friendId"] = chatId;
                // 能拿到文件描述符就是在线！
                if (GetOnlineFd(chatId) > 0)
                    singleObj["onlineState"] = true;
                else
                    singleObj["onlineState"] = false;
                _rootObj["onlineStateList"].append(singleObj);
            }
            _rootObj["state"] = true;
        }
        break;
    }
    case 816: // 查询所有好友/群聊昵称和头像
    {
        _rootObj["friendList"];
        _rootObj["groupList"];
        if (db->QueryFriendList(outValue, usrId))
        {
            _rootObj["friendList"] = outValue;
            _rootObj["state"] = true;
        }
        outValue.clear();
        if (db->QueryGroupList(outValue, usrId))
        {
            _rootObj["groupList"] = outValue;
            _rootObj["state"] = true;
        }
        break;
    }
    case 901: // 用户申请加好友/群聊✔
    {
        std::string ps = rootObj["ps"].asString();
        int type = rootObj["type"].asInt(); // 0-好友  1-群聊
        int reviewer;
        if (type)
        {
            // 入群申请
            int grpId = rootObj["groupId"].asInt();

            db->QueryGroupLeader(grpId, reviewer);
            if (db->AddUserApply(usrId, reviewer, 1, grpId, ps))
            {
                _rootObj["type"] = 1;
                _rootObj["state"] = true;
            }
        }
        else
        {
            // 好友申请
            reviewer = rootObj["friendId"].asInt();
            if (db->AddUserApply(usrId, reviewer, 0, 0, ps))
            {
                _rootObj["type"] = 0;
                _rootObj["state"] = true;
            }
        }
        // 先反馈给申请者
        Json::FastWriter writer;
        Send(fd, writer.write(_rootObj));

        // 用户在线直接发给用户去处理
        Json::Value replyJson;
        replyJson["code"] = 902;
        if (db->QueryApplyList(outValue, reviewer))
        {
            replyJson["applicantList"] = outValue;
            replyJson["state"] = true;
        }
        int frdFd = GetOnlineFd(reviewer);
        if (frdFd > 0)
            Send(frdFd, writer.write(replyJson));

        return; // 无需再次发送
    }
    case 902: // 用户申请列表✔
    {
        if (db->QueryApplyList(outValue, usrId))
        {
            _rootObj["applicantList"] = outValue;
            _rootObj["state"] = true;
        }
        break;
    }
    default:
        break;
    }

    // 序列化
    //  std::string json = _rootObj.toStyledString();
    Json::FastWriter writer;
    std::string json = writer.write(_rootObj);
    Send(fd, json); // 发送数据
}
