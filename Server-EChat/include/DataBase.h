#ifndef _DATABASE_H
#define _DATABASE_H

#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <thread>
#include <mutex>
#include <sstream>
#include <memory>

#include "json/json.h"

#define DEFAULT_HOST "127.0.0.1" // 默认主机
#define DEFAULT_USER "root"      // 默认用户
#define DEFAULT_PASSWORD "root"  // 默认密码
#define DEFAULT_DATABASE "echat" // 默认数据库

#define MSG_MAX_ROWS 100 // 默认读取历史消息行数

extern bool GetOnlineState(const int &usrId);
extern std::map<int, std::vector<int>> groups; // id - 群成员

class DataBase
{
public:
    static DataBase *GetInstance() { return instance; }

    bool ConnectDB(const std::string host = DEFAULT_HOST, const std::string usr = DEFAULT_USER,
                   const std::string pwd = DEFAULT_PASSWORD, const std::string db = DEFAULT_DATABASE);
    void CloseDB();

    // bool GenerateDiffID(int min, int max, int num); // 生成不重复ID-√
    bool getNewID(int &newId);    // 获取新Id(新Id表)-√
    bool DeleteNewID(int &newId); // 删除Id(新Id表)-√

    bool InitUI(Json::Value &outValue, const int &usrId); //

    bool AddUserRecommends(const int &usrId, std::string &text); // 用户建议

    bool QueryUser(const int &usrId);                                                        // 查询用户(用户表)-√
    bool Login(const int &usrId, const std::string &usrPwd);                                 // 验证用户(用户表)-√
    bool Register(const int &newId, const std::string &nickname, const std::string &usrPwd); // 注册用户(用户表)-√
    bool QueryFriendInfo(Json::Value &outValue, const int &usrId, const int &frdId);         // 查询好友详细信息(用户表)-√
    bool UpdateUserInfo(const int &usrId, Json::Value &rootObj);                             // 更新用户信息(用户表)-√
    bool UpdateUserPassword(const int &usrId, const std::string &newPwd);                    // 更新用户密码(用户表)-√
    bool UpdateUserPicture(const int &usrId, const int &picture);                            // 更新用户头像(用户表)-√
    bool QueryUserInfo(Json::Value &outValue, const int &usrId);                             // 查询用户信息(用户表)-√

    bool AddUserApply(const int &applicant, const int &reviewer,
                      const int &type, const int &grpId, const std::string &ps);                       // 添加好友/群聊申请(申请表)-√
    bool DeleteUserApply(const int &applicant, const int &reviewer, const int &type);                  // 删除好友/群聊申请(申请表)-√
    bool QueryUserApply(Json::Value &outValue, const int &usrId, const int &sideId, const bool &type); // 查询好友/用户申请(申请表)-√
    bool QueryApplyList(Json::Value &outValue, const int &usrId);                                      // 查询申请列表(申请表)-√

    bool AddFriend(const int &usrId, const int &frdId);                                      // 添加好友(好友表)-√
    bool DeleteFriend(const int &usrId, const int &frdId);                                   // 删除好友(好友表)-√
    bool QueryFriendRelation(std::string &outBuildTime, const int &usrId, const int &frdId); // 查询好友关系(好友表)-√
    bool QueryFriendList(Json::Value &outValue, const int &usrId);                           // 查询好友列表(好友表)-√

    bool QueryGroup(const int &grpId);                              // 查询群(群表)-√
    bool QueryGroupLeader(const int &grpId, int &outLeaderId);      // 查群主(群表)-√
    bool QueryGroupInfo(Json::Value &outValue, const int &grpId);   // 查询群信息(群表)-√
    bool QueryGroupMember(Json::Value &outValue, const int &grpId); // 查询群成员(群表)-√
    bool AddGroup(const int &leaderId, const int &grpId, const int &grpPicture, const std::string &groupName,
                  const std::string &members, const int &adminCount, const int &memberCount);               // 增加群(群表)-√
    bool DeleteGroup(const int &grpId);                                                                     // 删群(群表)-√
    bool UpdateGroupMember(const int &usrId, const int &grpId, const std::string &members);                 // 更新群成员(群表)-√
    bool UpdateGroupInfo(const int &grpId, const std::string &groupNickname, const std::string groupIntro); // 修改群消息(群表)-√
    bool UpdateGroupPicture(const int &grpId, const int &picture);                                          // 更新群头像(群表)-√

    bool AddUserInGroup(const int &usrId, const int &grpId, const std::string &identify);                   // 增加用户进群(用户进群表)-√
    bool DeleteUserInGroup(const int &usrId, const int &grpId);                                             // 删除用户进群数据(用户进群表)-√
    bool UpdateUserGroupNickname(const int &usrId, const int &grpId, const std::string &userGroupNickname); // 修改用户的群昵称(用户进群表)-√
    bool QueryUserGroupNickname(const int &usrId, const int &grpId, std::string &userGroupNickname);        // 查看用户的群昵称(用户进群表)-√
    bool QueryGroupRelation(Json::Value &outValue, const int &usrId, const int &grpId);                     // 查询群关系(用户进群表)-√
    bool QueryGroupList(Json::Value &outValue, const int &usrId);                                           // 查询群列表(用户进群表)-√

    bool AddOfflineMsg(const int &type, Json::Value &rootObj, int memberId = 0);                                                    // 增加聊天记录(离线消息表)-
    bool DeleteOfflineMsg(const int &usrId);                                                                                        // 删除好友聊天记录(离线消息表)-√
    bool QueryOfflineMsg(Json::Value &outValue, const int &sender, const int &receiver, const int &groupId, const int &startIndex); // 查询聊天信息(历史消息表)-!

    bool AddChatObject(const int &usrId, const int &sideId, const int &type);    // 增加聊天对象(聊天对象表)-√
    bool DeleteChatObject(const int &usrId, const int &sideId, const int &type); // 删除聊天对象(聊天对象表)-√
    bool QueryChatObject(const int &type, const int &usrId, const int &sideId);
    bool QueryChatObjectList(Json::Value &outValue, const int &usrId);   // 查询聊天对象(聊天对象表)-√
    bool QueryChatObjectIDList(Json::Value &outValue, const int &usrId); // 查询聊天对象ID(聊天对象表)-√

    bool UpdateConfig(const int &usrId);                       // 更新用户配置(配置表)-
    bool QueryConfig(Json::Value &outValue, const int &usrId); // 查询用户配置(配置表)-

private:
    DataBase();
    DataBase(const DataBase &) = delete;
    DataBase &operator=(const DataBase &) = delete;
    ~DataBase(){};

    void initGroupMap(); // 初始化群map

    static DataBase *instance;
    MYSQL *mysql;

    std::mutex mtx;

    // std::unique_lock<std::mutex> guard(mtx);
    // std::unique_lock<std::mutex> unique(mtx);
    // pthread_rwlock_t rwLock;
    // MYSQL_ROW row;
    // MYSQL_RES *result;
    // MYSQL_FIELD *field;
};

#endif
