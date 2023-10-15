
#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <string>
#include <map>
#include <list>
#include <vector>

// //
// std::vector<int> workFd;

// void AddWorkFd(const int &fd)
// {
//     workFd.push_back(fd);
// }

// void DeleteWorkFd(const int &fd)
// {
//     for (auto it = workFd.begin(); it != workFd.end(); ++it)
//     {
//         if (fd == *it)
//         {
//             workFd.erase(it);
//             break;
//         }
//     }
// }

// bool QueryWorkFd(const int &fd)
// {
//     for (auto it = workFd.begin(); it != workFd.end(); ++it)
//     {
//         if (fd == *it)
//             return true;
//     }
//     return false;
// }

// 在线用户
std::map<int, int> onlineFds; // id - fd

// 群成员
std::map<int, std::vector<int>> groups; // id - 群成员

// 添加在线用户
void AddOnlineFd(const int &usrId, const int &fd)
{
    // cout << "用户" << usrId << "上线 fd=" << fd << endl;
    onlineFds.insert(std::make_pair(usrId, fd));
}

// 删除在线用户
void DeleteOnlineFd(const int &fd)
{
    int usrId = -1;
    for (auto it = onlineFds.begin(); it != onlineFds.end(); ++it)
    {
        if (it->second != fd)
            continue;
        // cout << "用户" << it->first << "下线 fd=" << fd << endl;
        onlineFds.erase(it->first);
    }
}

void DeleteUser(const int &userId)
{
    onlineFds.erase(userId);
}

// 获取在线用户fd
int GetOnlineFd(const int &usrId)
{
    auto it = onlineFds.find(usrId);
    if (it != onlineFds.end())
        return it->second;
    return -1;
}

bool GetOnlineState(const int &usrId)
{
    auto it = onlineFds.find(usrId);
    if (it != onlineFds.end())
        return true;
    return false;
}

void ShowAllFd()
{
    for (auto it = onlineFds.begin(); it != onlineFds.end(); ++it)
    {
        cout << "用户" << it->first << "  fd=" << it->second << endl;
    }
}
