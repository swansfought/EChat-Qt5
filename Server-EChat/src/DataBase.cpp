
#include "DataBase.h"

DataBase *DataBase::instance = new DataBase;

DataBase::DataBase()
{
    mysql = mysql_init(nullptr);
    if (mysql == nullptr)
    {
        cout << "mysql_init Error: " << mysql_error(mysql);
        exit(-1);
    }
}

// 连接数据库
bool DataBase::ConnectDB(const std::string host, const std::string user, const std::string pwd, const std::string db)
{
    mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), pwd.c_str(), db.c_str(), 0, nullptr, 0);
    if (nullptr == mysql)
    {
        cout << "db error connecting: " << mysql_error(mysql) << endl;
        exit(-1);
    }
    // mysql_query成功查询返回的结果是0，设置utf-8编码，解决中文乱码问题
    if (mysql_query(mysql, "SET NAMES UTF8"))
    {
        cout << "SET NAMES UTF8 mysql_query failed: " << mysql_error(mysql);
    }
    cout << "[log]server successfully connected to db...\n";

    initGroupMap();
    return true;
}

// 关闭数据库连接
void DataBase::CloseDB()
{
    if (!mysql)
        mysql_close(this->mysql);
    mysql = nullptr;
    cout << "[log]db successfully closed..." << endl;
}

void DataBase::initGroupMap()
{
    char sql[100];

    // 拿到群的所有成员
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT grp_id,grp_leader,grp_members,grp_member_num FROM ec_groups");
    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "initDate mysql_query error:" << mysql_error(mysql) << endl;
        return;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "initDate mysql_store_result error:" << mysql_error(mysql) << endl;
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == nullptr)
    {
        mysql_free_result(res);
        res = nullptr;
        return;
    }

    int rows = mysql_num_rows(res);
    int fields = mysql_num_fields(res);
    for (int i = 0; i < rows; i++)
    {

        int grpId = atoi(row[0]); // 群ID
        int leaderId = atoi(row[1]);
        int memCount = atoi(row[3]); // 群成员数量

        std::vector<int> member;
        // member.resize(memCount + 1); // 1 为群主
        member.push_back(leaderId); // 添加群主

        // 添加群成员
        Json::Value outValue;
        if (QueryGroupMember(outValue, grpId))
        {
            if (!outValue["members"].isNull())
            {
                Json::Value memArr = outValue["members"];
                int counts = memArr.size();
                for (int i = 0; i < counts; i++)
                    member.push_back(memArr[i].asInt());
            }
        }
        row = mysql_fetch_row(res);

        groups.insert(make_pair(grpId, member)); // 插入群map中
    }

    mysql_free_result(res);
    res = nullptr;

    cout << "[log]group data init successful...\n";

#if 0
    for (auto it = groups.begin(); it != groups.end(); ++it)
    {
        cout << "群ID：" << it->first << endl;
        cout << "群成员：";
        auto _it = it->second;
        for (int i = 0; i < _it.size(); ++i)
        {
            cout << _it[i] << " ";
        }
        cout << endl;
    }
#endif
}

// 生成不重复ID --√
#if 0 // 已完成生成
bool DataBase::GenerateDiffID(int min, int max, int num)
{
    int diffID;
    std::vector<int> tmp;
    char sql[100];
    memset(sql, 0, sizeof(sql));

    for (int i = min; i < max + 1; i++)
        tmp.push_back(i);

    srand((unsigned)time(0)); // 初始化随机数种子
    for (int i = 0; i < num; i++)
    {
        do
        {
            diffID = min + rand() % (max - min + 1);

        } while (tmp.at(diffID - min) == -1);

        // sql
        sprintf(sql, "INSERT IGNORE ec_new_ids(new_id) VALUES(%d)", diffID);
        if (mysql_query(mysql, sql))
        {
            cout << "AddFriend mysql_query error:" << mysql_error(mysql);
            return false;
        }
        memset(sql, 0, sizeof(sql));

        tmp.at(diffID - min) = -1;
    }
    return true;
}
#endif

bool DataBase::InitUI(Json::Value &outValue, const int &usrId)
{
    Json::Value chatArr;
    Json::Value frdArr;
    // frdArr.append(usrId); // 自己在第一个
    Json::Value grpArr;
    char sql[100];

    // 聊天对象个数
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT frd_id,grp_id,type FROM ec_chat_objects WHERE usr_id=%d", usrId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "InitUI mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
        cout << "InitUI mysql_store_result error:" << mysql_error(mysql) << endl;
    else
    {
        int rows = mysql_num_rows(res);
        MYSQL_ROW row = mysql_fetch_row(res);
        for (int i = 0; i < rows; i++)
        {
            if (atoi(row[2]))
                chatArr.append(atoi(row[1]));
            else
                chatArr.append(atoi(row[0]));
            row = mysql_fetch_row(res);
        }
        mysql_free_result(res);
        res = nullptr;
    }

    // 好友个数
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT frd_id FROM ec_friends WHERE usr_id=%d", usrId);

    res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "InitUI mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
        cout << "InitUI mysql_store_result error:" << mysql_error(mysql) << endl;
    else
    {
        int rows = mysql_num_rows(res);
        MYSQL_ROW row = mysql_fetch_row(res);
        for (int i = 0; i < rows; i++)
        {
            frdArr.append(atoi(row[0]));
            row = mysql_fetch_row(res);
        }
        mysql_free_result(res);
        res = nullptr;
    }

    // 群聊个数
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT grp_id FROM ec_user_groups WHERE usr_id=%d", usrId);

    res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "InitUI mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
        cout << "InitUI mysql_store_result error:" << mysql_error(mysql) << endl;
    else
    {
        int rows = mysql_num_rows(res);
        MYSQL_ROW row = mysql_fetch_row(res);
        for (int i = 0; i < rows; i++)
        {
            grpArr.append(atoi(row[0]));
            row = mysql_fetch_row(res);
        }
        mysql_free_result(res);
        res = nullptr;
    }

    outValue["chatObjectList"] = chatArr;
    outValue["friendList"] = frdArr;
    outValue["groupList"] = grpArr;
    return true;
}

bool DataBase::AddUserRecommends(const int &usrId, std::string &text)
{
    std::ostringstream os;
    os << "INSERT ec_recommends(usr_id, usr_say) VALUES(" << usrId << ",'" << text.c_str() << "');";
    std::string sql = os.str();
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "AddUserRecommends mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    return true;
}

/********************************************************************************
 ********************************************************************************
 ************************      对[新Id表]的增删改查      *************************
 ********************************************************************************
 ********************************************************************************/
// 获取新Id(新Id表)
bool DataBase::getNewID(int &newId)
{
    if (QueryUser(newId))
        return false;
    std::lock_guard<std::mutex> guard(mtx);
    if (QueryUser(newId))
        return false;

    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT new_id FROM ec_new_ids ORDER BY RAND() LIMIT 1");
    if (mysql_query(mysql, sql))
    {
        cout << "SELECT getNewID mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "SELECT getNewID mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    mysql_free_result(res);
    res = nullptr;

    if (row == nullptr)
        return false;

    newId = atoi(row[0]); // 拿到新的id

    return true;
}

// 删除Id(新Id表)-√
bool DataBase::DeleteNewID(int &newId)
{
    // 把id从ec_new_ids表中移除
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "DELETE FROM ec_new_ids WHERE new_id=%d", newId);
    if (mysql_query(mysql, sql))
    {
        cout << "DeleteNewID mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    return true;
}
/********************************************************************************
 ********************************************************************************
 ***********************      对[用户在线表]的增删改查      **********************
 ********************************************************************************
 ********************************************************************************/
#if 0
// 查询用户在线状态(用户在线表)
bool DataBase::QueryOnlineState(const int &usrId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT usr_id FROM ec_online_users WHERE usr_id=%d", usrId);

    MYSQL_RES *onRes = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryOnlineState mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    onRes = mysql_store_result(mysql);
    if (nullptr == onRes)
    {
        cout << "QueryOnlineState mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW onRow = mysql_fetch_row(onRes);

    // 释放res资源
    mysql_free_result(onRes);
    onRes = nullptr;

    if (onRow <= 0)
        return false;
    return true;
}

// 添加用户在线(用户在线表)-√
bool DataBase::AddOnlineUser(const int &usrId)
{
    if (QueryOnlineState(usrId))
        return false;
    std::lock_guard<std::mutex> guard(mtx);

    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "INSERT IGNORE ec_online_users(usr_id) VALUES(%d)", usrId);

    if (mysql_query(mysql, sql))
    {
        cout << "AddOnlineUser  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 用户离线(用户在线表)-√
bool DataBase::DeleteOnlineUser(const int &usrId)
{
    if (!QueryOnlineState(usrId))
        return true;
    std::lock_guard<std::mutex> guard(mtx);
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "DELETE FROM ec_online_users WHERE usr_id=%d", usrId);

    if (mysql_query(mysql, sql))
    {
        cout << "AddOnlineUser  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}
#endif

/********************************************************************************
 ********************************************************************************
 ************************      对[用户表]的增删改查      *************************
 ********************************************************************************
 ********************************************************************************/
// 查询用户(用户表)-√
bool DataBase::QueryUser(const int &usrId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT usr_id FROM ec_users WHERE usr_id=%d", usrId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryUser mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryUser mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);

    // 释放res资源
    mysql_free_result(res);
    res = nullptr;

    if (row == nullptr)
        return false;
    return true;
}

// 查询好友详细信息(用户表)-√
bool DataBase::QueryFriendInfo(Json::Value &outValue, const int &usrId, const int &frdId)
{
    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT build_time,remark FROM ec_friends WHERE usr_id=%d AND frd_id=%d", usrId, frdId);

    MYSQL_RES *frdInfoRes = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryFriendInfo mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    frdInfoRes = mysql_store_result(mysql);
    if (nullptr == frdInfoRes)
    {
        cout << "QueryFriendInfo mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW frdRow = mysql_fetch_row(frdInfoRes); // 获取一行数据

    Json::Value userInfos;
    if (!QueryUserInfo(userInfos, frdId))
        return false;
    outValue["userInfos"] = userInfos;
    outValue["friendId"] = frdId;

    int fields = mysql_num_fields(frdInfoRes);
    for (int i = 0; i < fields; i++)
    {
        switch (i)
        {
        case 0:
        {
            if (nullptr == frdRow[i])
                outValue["buildTime"];
            else
                outValue["buildTime"] = frdRow[i];
            break;
        }
        case 1:
        {
            if (nullptr == frdRow[i])
                outValue["remark"];
            else
                outValue["remark"] = frdRow[i];
            break;
        }
        default:
            break;
        }
    }
    // 释放frdRes资源
    mysql_free_result(frdInfoRes);
    frdInfoRes = nullptr;

    return true;
}

// 验证用户(用户表)-√
bool DataBase::Login(const int &usrId, const std::string &userPwd)
{
    char sql[300];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT usr_id FROM ec_users WHERE usr_id=%d AND usr_password='%s'", usrId, userPwd.c_str());
    MYSQL_RES *verifyRes = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "Login mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    verifyRes = mysql_store_result(mysql);
    if (nullptr == verifyRes)
    {
        cout << "Login mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW verifyRow = mysql_fetch_row(verifyRes);

    // 释放res资源
    mysql_free_result(verifyRes);
    verifyRes = nullptr;

    if (verifyRow == nullptr)
        return false;
    return true;
}

// 注册用户(用户表)-✔
bool DataBase::Register(const int &newId, const std::string &nickname, const std::string &userPwd)
{
    if (QueryUser(newId))
        return false;
    std::lock_guard<std::mutex> guard(mtx);
    if (QueryUser(newId))
        return false;

    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "INSERT ec_users(usr_id,usr_password,usr_nickname) VALUES(%d,'%s','%s')", newId, userPwd.c_str(), nickname.c_str());

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "Register mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 修改用户信息(用户表)-
bool DataBase::UpdateUserInfo(const int &usrId, Json::Value &rootObj)
{
    const char *nickname;
    const char *sex;
    const char *phone;
    const char *region;
    const char *email;
    const char *selfSay;
    if (!rootObj["nickname"].isNull())
        nickname = rootObj["nickname"].asCString();
    else
        nickname = "";
    if (!rootObj["sex"].isNull())
        sex = rootObj["sex"].asCString();
    else
        sex = "";
    if (!rootObj["phone"].isNull())
        phone = rootObj["phone"].asCString();
    else
        phone = "";
    if (!rootObj["region"].isNull())
        region = rootObj["region"].asCString();
    else
        region = "";
    if (!rootObj["email"].isNull())
        email = rootObj["email"].asCString();
    else
        email = "";
    if (!rootObj["selfSay"].isNull())
        selfSay = rootObj["selfSay"].asCString();
    else
        selfSay = "";
    std::ostringstream os;
    os << "UPDATE ec_users SET usr_nickname='" << nickname
       << "',usr_sex='" << sex
       << "',usr_phone='" << phone
       << "',usr_region='" << region
       << "',usr_email='" << email
       << "',usr_self_say='" << selfSay << "'  "
       << "WHERE usr_id=" << usrId << ";";
    std::string sql = os.str();
    // cout << sql << endl;
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "UpdateUserInfo mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

bool DataBase::UpdateUserPassword(const int &usrId, const std::string &newPwd)
{
    std::ostringstream os;
    os << "UPDATE ec_users SET usr_password='" << newPwd.c_str() << "'  "
       << "WHERE usr_id=" << usrId << ";";
    std::string sql = os.str();
    cout << sql << endl;
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "UpdateUserPassword mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    return true;
}

// 更新用户头像(用户表)-
bool DataBase::UpdateUserPicture(const int &usrId, const int &picture)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "UPDATE ec_users SET usr_picture=%d WHERE usr_id=%d", picture, usrId);
    if (mysql_query(mysql, sql))
    {
        cout << "UpdateUserPassword mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    return true;
}

// 查询用户信息(用户表)-✔
bool DataBase::QueryUserInfo(Json::Value &outValue, const int &usrId)
{
    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT usr_nickname,usr_sex,usr_picture,usr_phone,usr_region,usr_email,usr_self_say FROM ec_users WHERE usr_id=%d", usrId);

    MYSQL_RES *userRes = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryFriendInfo mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    userRes = mysql_store_result(mysql);
    if (nullptr == userRes)
    {
        cout << "QueryFriendInfo mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW userRow = mysql_fetch_row(userRes); // 获取一行数据

    outValue["userId"] = usrId;
    int fields = mysql_num_fields(userRes); // 获取列数
    for (int k = 0; k < fields; k++)
    {
        switch (k)
        {
        case 0:
        {
            if (nullptr == userRow[k])
                outValue["nickname"];
            else
                outValue["nickname"] = userRow[k];
            break;
        }
        case 1:
        {
            if (nullptr == userRow[k])
                outValue["sex"];
            else
                outValue["sex"] = userRow[k];
            break;
        }
        case 2:
        {
            if (nullptr == userRow[k])
                outValue["picture"];
            else
                outValue["picture"] = atoi(userRow[k]);
            break;
        }
        case 3:
        {
            if (nullptr == userRow[k])
                outValue["phone"];
            else
                outValue["phone"] = atoi(userRow[k]);
            break;
        }
        case 4:
        {
            if (nullptr == userRow[k])
                outValue["region"];
            else
                outValue["region"] = userRow[k];
            break;
        }
        case 5:
        {
            if (nullptr == userRow[k])
                outValue["email"];
            else
                outValue["email"] = userRow[k];
            break;
        }
        case 6:
        {
            if (nullptr == userRow[k])
                outValue["self_say"];
            else
                outValue["self_say"] = userRow[k];
            break;
        }
        default:
            break;
        }
    }
    mysql_free_result(userRes);
    userRes = nullptr;
    return true;
}

/********************************************************************************
 ********************************************************************************
 *************************      对[申请表]的增删改查      ************************
 ********************************************************************************
 ********************************************************************************/
/**
 * 添加好友/群聊申请(申请表)-√
 * @param type 0-好友  1-群聊
 * @param grpId 好友申请该参数无意义
 */
bool DataBase::AddUserApply(const int &applicant, const int &reviewer, const int &type, const int &grpId, const std::string &ps)
{
    char sql[200];
    memset(sql, 0, sizeof(sql));
    if (type)
        sprintf(sql, "INSERT ec_apply(applicant,reviewer,type,grp_id,ps) VALUES (%d,%d,%d,%d,'%s')", applicant, reviewer, 1, grpId, ps.c_str());
    else
        sprintf(sql, "INSERT ec_apply(applicant,reviewer,type,grp_id,ps) VALUES (%d,%d,%d,null,'%s')", applicant, reviewer, 0, ps.c_str());

    if (mysql_query(mysql, sql))
    {
        cout << "AddUserApply  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

/**
 *  删除好友/群聊申请(申请表)-√
 * @param type 0-好友  1-群聊 不要搞混!!!
 * @param grpId 好友申请该参数无意义
 */
bool DataBase::DeleteUserApply(const int &applicant, const int &reviewer, const int &type)
{
    // std::lock_guard<std::mutex> guard(mtx);

    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "DELETE FROM ec_apply WHERE applicant=%d AND reviewer=%d AND type=%d", applicant, reviewer, type);

    if (mysql_query(mysql, sql))
    {
        cout << "DeleteUserApply  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

/**
 * 查询好友/用户申请-√
 * @param type 0-好友  1-群聊
 */
bool DataBase::QueryUserApply(Json::Value &outValue, const int &usrId, const int &sideId, const bool &type)
{
    char sql[150];
    memset(sql, 0, sizeof(sql));
    if (type)
        sprintf(sql, "SELECT reviewer,grp_id,apply_time,ps FROM ec_apply WHERE applicant=%d AND grp_id=%d AND type=%d", usrId, sideId, 1);
    else
        sprintf(sql, "SELECT reviewer,apply_time,ps FROM ec_apply WHERE applicant=%d AND reviewer=%d AND type=%d", usrId, sideId, 0);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryUserApply mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryUserApply mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); // 获取一行数据

    if (row == nullptr)
        return false;
    outValue["applyTime"];
    outValue["ps"];
    if (type)
    {
        if (nullptr != row[2])
            outValue["applyTime"] = row[2];
        if (nullptr != row[3])
            outValue["ps"] = row[3];
    }
    else
    {
        if (nullptr != row[1])
            outValue["applyTime"] = row[1];
        if (nullptr != row[2])
            outValue["ps"] = row[2];
    }
    mysql_free_result(res);
    res = nullptr;
    return true;
}

// 查询申请列表(申请表)-√
bool DataBase::QueryApplyList(Json::Value &outValue, const int &usrId)
{
    char sql[150];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT applicant,type,grp_id,ps FROM ec_apply WHERE reviewer=%d", usrId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryApplyList mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryApplyList mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res); // 获取一行数据
    if (row == nullptr)
    {
        mysql_free_result(res);
        res = nullptr;
        return false;
    }

    int rows = mysql_num_rows(res);
    int fields = mysql_num_fields(res);
    Json::Value arr;
    for (int i = 0; i < rows; i++)
    {
        Json::Value single;
        for (int i = 0; i < fields; i++)
        {
            switch (i)
            {
            case 0:
            {
                int applicant = atoi(row[0]);
                single["applicantId"] = applicant;
                Json::Value usrInfo;
                if (QueryUserInfo(usrInfo, applicant))
                {
                    single["nickname"] = usrInfo["nickname"];
                    single["picture"] = usrInfo["picture"];
                }
                break;
            }
            case 1:
            {
                single["type"] = atoi(row[1]);
                break;
            }
            case 2:
            {
                if (atoi(row[1]))
                    single["groupId"] = atoi(row[2]);
                break;
            }
            case 3:
            {
                if (nullptr == row[3])
                    single["ps"];
                else
                    single["ps"] = row[3];
                break;
            }
            default:
                break;
            }
        }
        arr.append(single);
        row = mysql_fetch_row(res);
    }
    outValue = arr;

    mysql_free_result(res);
    res = nullptr;

    return true;
}

/********************************************************************************
 ********************************************************************************
 *************************      对[好友表]的增删改查      ************************
 ********************************************************************************
 ********************************************************************************/
// 添加好友(好友表)-√
bool DataBase::AddFriend(const int &usrId, const int &frdId)
{
    std::lock_guard<std::mutex> guard(mtx);

    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "INSERT ec_friends(usr_id,frd_id) VALUES(%d,%d)", usrId, frdId);
    if (mysql_query(mysql, sql))
    {
        cout << "AddFriend mysql_query error:" << mysql_error(mysql);
        return false;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "INSERT ec_friends(usr_id,frd_id) VALUES(%d,%d)", frdId, usrId);
    if (mysql_query(mysql, sql))
    {
        cout << "AddFriend mysql_query error:" << mysql_error(mysql);
        return false;
    }

    return true;
}

// 删除好友(好友表)-√
bool DataBase::DeleteFriend(const int &usrId, const int &frdId)
{
    std::lock_guard<std::mutex> guard(mtx);

    char sql[150];
    // 好友关系
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "DELETE FROM ec_friends WHERE usr_id=%d AND frd_id=%d", usrId, frdId);
    if (mysql_query(mysql, sql))
    {
        cout << "DeleteFriend mysql_query error:" << mysql_error(mysql);
        return false;
    }

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "DELETE FROM ec_friends WHERE usr_id=%d AND frd_id=%d", frdId, usrId);
    if (mysql_query(mysql, sql))
    {
        cout << "DeleteFriend mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 查询好友关系(好友表)-√
bool DataBase::QueryFriendRelation(std::string &outBuildTime, const int &usrId, const int &frdId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT build_time FROM ec_friends WHERE usr_id=%d AND frd_id=%d", usrId, frdId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryFriendRelation mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryFriendRelation mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);

    // 释放res资源
    mysql_free_result(res);
    res = nullptr;

    if (row == nullptr)
        return false;

    outBuildTime = row[0]; // 建立时间
    return true;
}

// 查询好友列表(好友表)-√
bool DataBase::QueryFriendList(Json::Value &outValue, const int &usrId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT frd_id FROM ec_friends WHERE usr_id=%d", usrId);

    MYSQL_RES *frdRes = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryContactList mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    frdRes = mysql_store_result(mysql);
    if (nullptr == frdRes)
    {
        cout << "QueryContactList mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }

    MYSQL_ROW frdRow = mysql_fetch_row(frdRes); // 获取一行数据
    if (frdRow  == nullptr)
    {
        mysql_free_result(frdRes);
        frdRes = nullptr;
        return false;
    }

    int rows = mysql_num_rows(frdRes);
    Json::Value arr;
    for (int i = 0; i < rows; i++)
    {
        Json::Value frdInfo;
        Json::Value userInfo;
        int frdId = atoi(frdRow[0]);
        if (QueryUserInfo(userInfo, frdId))
        {
            frdInfo["picture"] = userInfo["picture"];
            frdInfo["nickname"] = userInfo["nickname"];
        }
        frdInfo["friendId"] = frdId;
        arr.append(frdInfo);
        frdRow = mysql_fetch_row(frdRes);
    }
    outValue = arr;

    mysql_free_result(frdRes);
    frdRes = nullptr;

    return true;
}

/********************************************************************************
 ********************************************************************************
 *************************      对[群表]的增删改查        ************************
 ********************************************************************************
 ********************************************************************************/
// 查询群(群表)-√
bool DataBase::QueryGroup(const int &grpId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT grp_id FROM ec_groups WHERE grp_id=%d", grpId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryGroup mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryGroup mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);

    // 释放res资源
    mysql_free_result(res);
    res = nullptr;

    if (row == nullptr)
        return false;
    return true;
}

// 查群主(群表)-√
bool DataBase::QueryGroupLeader(const int &grpId, int &outLeaderId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT grp_leader FROM ec_groups WHERE grp_id=%d", grpId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryLeader mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryLeader mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);

    // 释放res资源
    mysql_free_result(res);
    res = nullptr;

    if (row == nullptr)
        return false;
    outLeaderId = atoi(row[0]);
    return true;
}

// 查询群信息(群表)-√
bool DataBase::QueryGroupInfo(Json::Value &outValue, const int &grpId)
{
    char sql[300];
    memset(sql, 0, sizeof(sql));
    sprintf(sql,
            "SELECT grp_leader,grp_nickname,grp_build_time,grp_picture,grp_intro,grp_admin_num,grp_member_num FROM ec_groups WHERE grp_id=%d",
            grpId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryGroupInfo mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryGroupInfo mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); // 获取一行数据

    outValue["groupId"] = grpId;
    int fields = mysql_num_fields(res); // 获取列数
    for (int k = 0; k < fields; k++)
    {
        switch (k)
        {
        case 0:
        {
            if (nullptr == row[k])
                outValue["leaderId"];
            else
            {
                int leaderId = atoi(row[k]);
                outValue["leaderId"] = leaderId;
                Json::Value outTmp;
                if (QueryGroupRelation(outTmp, leaderId, grpId))
                {
                    if (!outTmp["userGroupNickname"].isNull())
                        outValue["leaderNickname"] = outTmp["userGroupNickname"];
                    else
                    {
                        outTmp.clear();
                        if (QueryUserInfo(outTmp, leaderId))
                            outValue["leaderNickname"] = outTmp["nickname"];
                    }
                }
                outTmp.clear();
                if (QueryUserInfo(outTmp, leaderId))
                {
                    outValue["leaderPicture"] = outTmp["picture"];
                }
            }
            break;
        }
        case 1:
        {
            if (nullptr == row[k])
                outValue["nickname"];
            else
                outValue["nickname"] = row[k];
            break;
        }
        case 2:
        {
            if (nullptr == row[k])
                outValue["buildTime"];
            else
                outValue["buildTime"] = row[k];
            break;
        }
        case 3:
        {
            if (nullptr == row[k])
                outValue["picture"];
            else
                outValue["picture"] = atoi(row[k]);
            break;
        }
        case 4:
        {
            if (nullptr == row[k])
                outValue["intro"];
            else
                outValue["intro"] = row[k];
            break;
        }
        case 5:
        {
            if (nullptr == row[k])
                outValue["adminCount"] = 0;
            else
                outValue["adminCount"] = atoi(row[k]);
            break;
        }
        case 6:
        {
            if (nullptr == row[k])
                outValue["memberCount"] = 0;
            else
                outValue["memberCount"] = atoi(row[k]);
            break;
        }
        default:
            break;
        }
    }
    mysql_free_result(res);
    res = nullptr;
    return true;
}

// 查询群成员(群表)-√
bool DataBase::QueryGroupMember(Json::Value &outValue, const int &grpId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT grp_members FROM ec_groups WHERE grp_id=%d", grpId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryGroupMember mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryGroupMember mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); // 获取一行数据
    if (row == nullptr)
    {
        mysql_free_result(res);
        res = nullptr;
        return false;
    }

    // 拿到群成员json数组
    if (nullptr == row[0])
        outValue["members"];
    else
    {
        // 解析成员数组
        Json::Reader reader;
        Json::Value memArr;
        Json::Value tmpArr;
        reader.parse(row[0], memArr);
        for (int i = 0; i < memArr.size(); ++i)
        {
            tmpArr.append(memArr[i]);
        }
        outValue["members"] = tmpArr;
    }

    return true;
}

// 增加群(群表)-√
bool DataBase::AddGroup(const int &leaderId, const int &grpId, const int &grpPicture, const std::string &groupName,
                        const std::string &members, const int &adminCount, const int &memberCount)
{
    std::lock_guard<std::mutex> guard(mtx);
    bool state = true;

    std::ostringstream os;
    os << "INSERT ec_groups(grp_id,grp_leader,grp_picture,grp_nickname,grp_members,grp_admin_num,grp_member_num) VALUES ("
       << grpId << "," << leaderId << "," << grpPicture << ",'" << groupName.c_str()
       << "','" << members.c_str() << "'," << adminCount << "," << memberCount << ")";
    std::string sql = os.str();
    // cout << sql << endl;
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "AddGroup ec_groups mysql_query error:" << mysql_error(mysql);
        state = false;
    }
    return state;
}

// 删群(群表)-√
bool DataBase::DeleteGroup(const int &grpId)
{
    std::lock_guard<std::mutex> guard(mtx);
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, " DELETE FROM ec_groups WHERE grp_id=%d", grpId);
    if (mysql_query(mysql, sql))
    {
        cout << "DeleteGroup  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 更新群成员(群表)-√
bool DataBase::UpdateGroupMember(const int &usrId, const int &grpId, const std::string &members)
{
    std::lock_guard<std::mutex> guard(mtx);
    bool state = true;

    std::ostringstream os;
    os << "UPDATE ec_groups SET grp_members='" << members.c_str() << "' WHERE grp_id=" << grpId;
    std::string sql = os.str();
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "UpdateGroupMember mysql_query error:" << mysql_error(mysql);
        state = false;
    }
    return state;
}

// 修改群消息(群表)-√
bool DataBase::UpdateGroupInfo(const int &grpId, const std::string &groupNickname, const std::string groupIntro)
{
    std::lock_guard<std::mutex> guard(mtx);
    bool state = true;

    std::ostringstream os;
    os << "UPDATE ec_groups SET grp_nickname='" << groupNickname.c_str() << "',grp_intro='" << groupIntro.c_str() << "' WHERE grp_id=" << grpId;
    std::string sql = os.str();
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "UpdateGroupInfo  mysql_query error:" << mysql_error(mysql);
        state = false;
    }
    return state;
}

// 更新群头像(群表)-√
bool DataBase::UpdateGroupPicture(const int &grpId, const int &picture)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "UPDATE ec_groups SET grp_picture=%d WHERE grp_id=%d", picture, grpId);
    if (mysql_query(mysql, sql))
    {
        cout << "UpdateGroupPicture mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    return true;
}

/********************************************************************************
 ********************************************************************************
 *************************   对[用户进群表]的增删改查     ************************
 ********************************************************************************
 ********************************************************************************/
// 增加用户进群(用户进群表)-√
bool DataBase::AddUserInGroup(const int &usrId, const int &grpId, const std::string &identify)
{
    std::lock_guard<std::mutex> guard(mtx);

    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "INSERT ec_user_groups(usr_id,grp_id,identify) VALUES (%d,%d,'%s')", usrId, grpId, identify.c_str());
    if (mysql_query(mysql, sql))
    {
        cout << "AddUserInGroup  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 删除用户进群(用户进群表)-√
bool DataBase::DeleteUserInGroup(const int &usrId, const int &grpId)
{
    std::lock_guard<std::mutex> guard(mtx);

    char sql[100];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "DELETE FROM ec_user_groups WHERE usr_id=%d AND grp_id=%d", usrId, grpId);
    if (mysql_query(mysql, sql))
    {
        cout << "DeleteUserInGroup  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 修改用户群昵称(用户进群表)-√
bool DataBase::UpdateUserGroupNickname(const int &usrId, const int &grpId, const std::string &userGroupNickname)
{
    std::lock_guard<std::mutex> guard(mtx);

    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "UPDATE ec_user_groups SET usr_groupNickname='%s' WHERE usr_id=%d AND grp_id=%d", userGroupNickname.c_str(), usrId, grpId);
    if (mysql_query(mysql, sql))
    {
        cout << "UpdateUserGroupNickname  mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 查询群列表(用户进群表)-√
bool DataBase::QueryGroupList(Json::Value &outValue, const int &usrId)
{
    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT grp_id FROM ec_user_groups WHERE usr_id=%d", usrId);
    MYSQL_RES *grpRes = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryContactList mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    grpRes = mysql_store_result(mysql);
    if (nullptr == grpRes)
    {
        cout << "QueryContactList mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }

    MYSQL_ROW grpRow = mysql_fetch_row(grpRes); // 获取一行数据
    if (grpRow  == nullptr)
    {
        mysql_free_result(grpRes);
        grpRes = nullptr;
        return false;
    }

    int rows = mysql_num_rows(grpRes);
    Json::Value arr;
    for (int i = 0; i < rows; i++)
    {
        Json::Value grpInfo;
        Json::Value subObj;
        int grpId = atoi(grpRow[0]);
        if (QueryGroupInfo(subObj, grpId))
        {
            grpInfo["picture"] = subObj["picture"];
            grpInfo["nickname"] = subObj["nickname"];
        }
        grpInfo["groupId"] = grpId;

        arr.append(grpInfo);
        grpRow = mysql_fetch_row(grpRes); // 获取一行数据
    }
    outValue = arr;

    mysql_free_result(grpRes);
    grpRes = nullptr;

    return true;
}

// 查看用户的群昵称(用户进群表)-√
bool DataBase::QueryUserGroupNickname(const int &usrId, const int &grpId, std::string &userGroupNickname)
{
    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT usr_groupNickname FROM ec_user_groups WHERE usr_id=%d AND grp_id=%d;", usrId, grpId);
    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryUserGroupNickname mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryUserGroupNickname mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res); // 获取一行数据
    if (row == nullptr)
    {
        mysql_free_result(res);
        res = nullptr;
        return false;
    }
    if (row[0] == nullptr)
        return false;

    userGroupNickname = row[0];
    return true;
}

// 查询群关系(用户进群表)-√
bool DataBase::QueryGroupRelation(Json::Value &outValue, const int &usrId, const int &grpId)
{
    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT usr_groupNickname,join_time FROM ec_user_groups WHERE usr_id=%d AND grp_id=%d", usrId, grpId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryFriendRelation mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryFriendRelation mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);

    if (row == nullptr)
    {
        mysql_free_result(res);
        res = nullptr;
        return false;
    }

    int rows = mysql_num_rows(res);
    if (nullptr == row[0])
        outValue["userGroupNickname"];
    else
        outValue["userGroupNickname"] = row[0]; // 用户群昵称

    if (nullptr == row[1])
        outValue["joinTime"];
    else
        outValue["joinTime"] = row[1]; // 加入时间
    mysql_free_result(res);
    res = nullptr;
    return true;
}

/********************************************************************************
 ********************************************************************************
 *************************** 对[历史消息表]的增删改查   ***************************
 ********************************************************************************
 ********************************************************************************/
// 增加聊天记录(历史消息表)-
bool DataBase::AddOfflineMsg(const int &type, Json::Value &rootObj, int memberId)
{
    // std::lock_guard<std::mutex> guard(mtx);

    std::string msgType = rootObj["msgType"].asString();
    std::string message = rootObj["message"].asString();
    std::string sendTime = rootObj["sendTime"].asString();
    int sender = rootObj["sender"].asInt();
    int senderPicture = rootObj["senderPicture"].asInt();
    int receiver = rootObj["receiver"].asInt();

    std::ostringstream os;
    std::string sql;
    // 普通信息
    if ("text" == msgType)
    {
        // 群聊
        if (type)
        {
            os << "INSERT ec_offline_messages(sender,frd_id,grp_id,type,message,msg_type,send_time,senderPicture) VALUES("
               << sender << "," << memberId << "," << receiver << "," << type << ",'"
               << message << "','" << msgType << "','" << sendTime << "'," << senderPicture << ")";
        }
        else
        { // 好友
            os << "INSERT ec_offline_messages(sender,frd_id,type,message,msg_type,send_time,senderPicture) VALUES("
               << sender << "," << receiver << "," << type << ",'" << message << "','"
               << msgType << "','" << sendTime << "'," << senderPicture << ")";
        }
        sql = os.str();
        if (mysql_query(mysql, sql.c_str()))
        {
            cout << "AddOfflineMsg [text] mysql_query error:" << mysql_error(mysql) << endl;
            return false;
        }
        return true;
    }

    // 图片/文件
    if ("image" == msgType || "file" == msgType)
    {
        const char *fineName = rootObj["fileName"].asCString();
        const char *suffix = rootObj["suffix"].asCString();
        const char *filePath = rootObj["filePath"].asCString();
        if (type)
        {
            os << "INSERT ec_offline_messages(sender,grp_id,type,message,msg_type,file_name,suffix,file_path) VALUES("
               << sender << "," << receiver << "," << type << ",'" << rootObj["message"].asString() << "','" << msgType << "','"
               << fineName << "','" << suffix << "','" << filePath << "')";
        }
        else
        {
            os << "INSERT ec_offline_messages(sender,frd_id,type,message,msg_type,file_name,suffix,file_path) VALUES("
               << sender << "," << receiver << "," << type << ",'" << rootObj["message"].asString() << "','" << msgType << "','"
               << fineName << "','" << suffix << "','" << filePath << "')";
        }
        sql = os.str();

        if (mysql_query(mysql, sql.c_str()))
        {
            cout << "AddOfflineMsg [image/file] mysql_query error:" << mysql_error(mysql) << endl;
            return false;
        }
        return true;
    }
    return false;
}

// 删除好友聊天记录(历史消息表)-√
bool DataBase::DeleteOfflineMsg(const int &usrId)
{
    std::lock_guard<std::mutex> guard(mtx);
    char sql[200];

    // 离线消息
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "DELETE FROM ec_offline_messages WHERE frd_id=%d;", usrId);
    if (mysql_query(mysql, sql))
    {
        cout << "DeleteOfflineMsg mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 查询聊天记录(历史消息表)-
// 如果是好友，发送者和接收者ID可以互换，不影响
// 但是如果是群，发送者和接收者ID不可互换！！！
// 查询的结果也是两者互发的结果
bool DataBase::QueryOfflineMsg(Json::Value &outValue, const int &sender, const int &receiver, const int &groupId, const int &startIndex)
{
    std::ostringstream os;
    std::string sql;

    // 好友
    if (!groupId)
    {
        os << "SELECT * FROM ec_offline_messages WHERE sender="
           << sender << " AND frd_id="
           << receiver << " "
           << " ORDER BY send_time  "           // DESC
           << " LIMIT " << startIndex << ",50"; // 50条数据一限制
    }
    else
    { // 群聊
        os << "SELECT * FROM ec_offline_messages WHERE frd_id="
           << receiver << " AND grp_id="
           << groupId << " "
           << " ORDER BY send_time  "            // DESC
           << " LIMIT " << startIndex << ",100"; // 100条数据一限制
    }
    sql = os.str();
    // cout << "sql = " << sql << endl;

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "QueryOfflineMsg mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryOfflineMsg mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res); // 获取一行数据
    if (row == nullptr)
    {
        mysql_free_result(res);
        res = nullptr;
        return false;
    }

    int rows = mysql_num_rows(res);     // 行数
    int fields = mysql_num_fields(res); // 列数
    // cout << "rows: " << rows << endl;
    // cout << "fields: " << fields << endl;
    for (int i = 0; i < rows; i++)
    {
        Json::Value singleMsg;
        std::string msgType;
        for (int j = 1; j < fields; j++)
        {
            switch (j)
            {
            case 1:
            {
                int sender = atoi(row[j]);
                singleMsg["sender"] = sender;
                // 方便群聊知道个人必要信息
                if (groupId)
                {
                    // 拿到发送者昵称和头像
                    singleMsg["senderInfo"];
                    Json::Value subObj;
                    Json::Value senderInfo;
                    if (QueryUserInfo(subObj, sender))
                    {
                        std::string userGroupNickname;
                        senderInfo["nickname"] = subObj["nickname"];
                        senderInfo["picture"] = subObj["picture"];
                        singleMsg["senderInfo"] = senderInfo;

                        // 查询用户群昵称
                        if (QueryUserGroupNickname(sender, receiver, userGroupNickname))
                            senderInfo["nickname"] = userGroupNickname;
                    }
                }
                break;
            }
            case 2:
            case 3:
            case 4:
                break;
            case 5:
            {
                singleMsg["message"] = row[j];
                break;
            }
            case 6:
            {
                msgType = row[j];
                singleMsg["msgType"] = msgType;
            }
            case 7:
            {
                singleMsg["sendTime"] = row[j];
                break;
            }
            case 8:
            {
                if ("text" == msgType)
                    break;
                singleMsg["fileName"] = row[j]; // 不做文件消息的非空判断了，是文件就必须得存这些信息！
                break;
            }
            case 9:
            {
                if ("text" == msgType)
                    break;
                singleMsg["suffix"] = row[j];
                break;
            }
            case 10:
            {
                if ("text" == msgType)
                    break;
                singleMsg["filePath"] = row[j];
                break;
            }
            case 11: // 发送者头像
            {
                break;
            }
            default:
                break;
            }
        }
        outValue.append(singleMsg);
        row = mysql_fetch_row(res);
        // cout << "outValue=" << outValue << endl;
    }
    mysql_free_result(res);
    res = nullptr;
    return true;
}

/********************************************************************************
 ********************************************************************************
 *************************    对[聊天用户表]的增删改查    ************************
 ********************************************************************************
 ********************************************************************************/
/**
 * 增加聊天对象(聊天对象表)-√
 * @param {type} 0-好友  1-群id
 */
bool DataBase::AddChatObject(const int &usrId, const int &sideId, const int &type)
{
    // 已存在无需添加
    if (QueryChatObject(type, usrId, sideId))
        return true;

    char sql[150];
    memset(sql, 0, sizeof(sql)); // IGNORE
    if (type)
        sprintf(sql, "INSERT IGNORE ec_chat_objects(usr_id,grp_id,type,chat_top) VALUES(%d,%d,%d,0)", usrId, sideId, type);
    else
        sprintf(sql, "INSERT IGNORE ec_chat_objects(usr_id,frd_id,type,chat_top) VALUES(%d,%d,%d,0)", usrId, sideId, type);

    if (mysql_query(mysql, sql))
    {
        cout << "AddChatObject mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

/**
 * 删除聊天对象(聊天对象表)-√
 * @param {type} 0-好友  1-群id
 */
bool DataBase::DeleteChatObject(const int &usrId, const int &sideId, const int &type)
{
    char sql[150];
    memset(sql, 0, sizeof(sql));
    if (type)
        sprintf(sql, "DELETE FROM ec_chat_objects WHERE usr_id=%d AND grp_id=%d", usrId, sideId);
    else
        sprintf(sql, "DELETE FROM ec_chat_objects WHERE usr_id=%d AND frd_id=%d", usrId, sideId);

    if (mysql_query(mysql, sql))
    {
        cout << "DeleteChatObject mysql_query error:" << mysql_error(mysql);
        return false;
    }
    return true;
}

// 查询聊天对象
bool DataBase::QueryChatObject(const int &type, const int &usrId, const int &sideId)
{
    char sql[100];
    memset(sql, 0, sizeof(sql));
    if (type)
        sprintf(sql, "SELECT usr_id FROM ec_chat_objects WHERE grp_id=%d AND usr_id=%d  AND type=1", sideId, usrId);
    else
        sprintf(sql, "SELECT usr_id FROM ec_chat_objects WHERE frd_id=%d AND usr_id=%d AND type=0", sideId, usrId);

    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryChatObject mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
    {
        cout << "QueryChatObject mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);

    // 释放res资源
    mysql_free_result(res);
    res = nullptr;

    if (row == nullptr)
        return false;
    return true;
}

// 查询聊天对象列表-√
bool DataBase::QueryChatObjectList(Json::Value &outValue, const int &usrId)
{
    char sql[200];
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT frd_id,grp_id,type,recent_chat_time,chat_top FROM ec_chat_objects WHERE usr_id=%d", usrId);

    MYSQL_RES *chatObjRes = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryChatObjectList mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    chatObjRes = mysql_store_result(mysql);
    if (nullptr == chatObjRes)
    {
        cout << "QueryChatObjectList mysql_store_result error:" << mysql_error(mysql) << endl;
        return false;
    }
    MYSQL_ROW chatObjectRow = mysql_fetch_row(chatObjRes);

    // 没有聊天对象
    if (chatObjectRow == nullptr)
    {
        // 释放chatObjRes资源
        mysql_free_result(chatObjRes);
        chatObjRes = nullptr;
        return true;
    }
    // 有聊天对象
    int rows = mysql_num_rows(chatObjRes);
    int fields = mysql_num_fields(chatObjRes);

    for (int i = 0; i < rows; i++)
    {
        int type = atoi(chatObjectRow[2]);
        Json::Value singleObj;
        for (int j = 0; j < fields; j++)
        {
            switch (j)
            {
            case 0:
            {
                if (type)
                    break;
                // 好友对象
                singleObj["type"] = 0;
                int frdId = atoi(chatObjectRow[j]);
                singleObj["friendId"] = frdId;

                // 拿到好友昵称和头像
                Json::Value subObj;
                if (QueryUserInfo(subObj, frdId))
                {
                    singleObj["nickname"] = subObj["nickname"];
                    singleObj["picture"] = subObj["picture"];
                }
                // 拿到好友在线状态
                if (GetOnlineState(frdId))
                    singleObj["onlineState"] = true;
                else
                    singleObj["onlineState"] = false;
                break;
            }
            case 1:
            {
                if (!type)
                    break;
                // 群对象
                singleObj["type"] = 1;
                int grpId = atoi(chatObjectRow[j]);
                singleObj["groupId"] = grpId;

                // 拿到群昵称和头像
                Json::Value subObj; // 用于接收查询返回值
                if (QueryGroupInfo(subObj, grpId))
                {
                    singleObj["nickname"] = subObj["nickname"];
                    singleObj["picture"] = subObj["picture"];
                    subObj.clear();
                }

                // 获取群主和群成员信息
                singleObj["memberInfos"];
                int leaderId;
                if (QueryGroupLeader(grpId, leaderId))
                {
                    Json::Value leaderInfo;
                    if (QueryUserInfo(subObj, leaderId))
                    {
                        leaderInfo["userId"] = leaderId;
                        leaderInfo["nickname"] = subObj["nickname"];
                        leaderInfo["picture"] = subObj["picture"];
                        if (GetOnlineState(leaderId))
                            leaderInfo["onlineState"] = true;
                        else
                            leaderInfo["onlineState"] = false;
                        singleObj["memberInfos"].append(leaderInfo);
                    }
                }

                Json::Value allMemArr;
                if (QueryGroupMember(allMemArr, grpId))
                {
                    if (allMemArr["members"].isNull())
                        break;
                    Json::Value memArr;
                    int counts = allMemArr["members"].size();
                    singleObj["adminCount"] = 1;
                    singleObj["memberCount"] = counts;
                    for (int i = 0; i < counts; ++i)
                    {
                        Json::Value memInfo;
                        Json::Value userInfo;
                        int userId = allMemArr["members"][i].asInt();

                        if (QueryUserInfo(userInfo, userId))
                        {
                            memInfo["userId"] = userId;
                            memInfo["nickname"] = userInfo["nickname"];
                            memInfo["picture"] = userInfo["picture"];
                        }
                        if (GetOnlineState(userId))
                            memInfo["onlineState"] = true;
                        else
                            memInfo["onlineState"] = false;
                        singleObj["memberInfos"].append(memInfo);
                        // memArr.append(memInfo);
                    }
                    // singleObj["memberInfos"] = memArr;
                }
                break;
            }
            case 3:
            {
                if (nullptr == chatObjectRow[j])
                    singleObj["recentChatTime"];
                else
                    singleObj["recentChatTime"] = chatObjectRow[j];
                break;
            }
            case 4:
            {
                if (nullptr == chatObjectRow[j])
                    singleObj["chatTop"];
                else
                    singleObj["chatTop"] = chatObjectRow[j];
                break;
            }
            default:
                break;
            }
        }
        outValue.append(singleObj);
        chatObjectRow = mysql_fetch_row(chatObjRes); // 获取一行数据
    }
    // 释放frdRes资源
    mysql_free_result(chatObjRes);
    chatObjRes = nullptr;

    return true;
}

// 查询聊天对象ID-√
bool DataBase::QueryChatObjectIDList(Json::Value &outValue, const int &usrId)
{
    char sql[100];

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "SELECT frd_id,grp_id,type FROM ec_chat_objects WHERE usr_id=%d", usrId);
    MYSQL_RES *res = nullptr;
    if (mysql_query(mysql, sql))
    {
        cout << "QueryChatObjectIDList mysql_query error:" << mysql_error(mysql) << endl;
        return false;
    }
    res = mysql_store_result(mysql);
    if (nullptr == res)
        cout << "QueryChatObjectIDList mysql_store_result error:" << mysql_error(mysql) << endl;
    else
    {
        int rows = mysql_num_rows(res);
        MYSQL_ROW row = mysql_fetch_row(res);
        for (int i = 0; i < rows; i++)
        {
            if (atoi(row[2]))
                outValue.append(atoi(row[1]));
            else
                outValue.append(atoi(row[0]));
            row = mysql_fetch_row(res);
        }
    }
    mysql_free_result(res);
    res = nullptr;
    return true;
}

/********************************************************************************
 ********************************************************************************
 *************************      对[配置表]的增删改查      ************************
 ********************************************************************************
 ********************************************************************************/
// 查询用户配置(配置表)-
bool DataBase::QueryConfig(Json::Value &outValue, const int &usrId)
{
    return false;
}

// 修改用户配置(配置表)-
bool DataBase::UpdateConfig(const int &usrId)
{
    return false;
}