#ifndef UIRELY_H
#define UIRELY_H

#include "thumbnail.h"

enum class MsgType{
    Text,
    File,
    Image
};

enum class ClikedType{
    FriendApply,
    AddContact,
    Self,
    AddGroup,
    Friend,
    Group,
    ChatObject
};

enum class ButtonType{
    Friend,
    Group,
    ChatObject
};

#define FRIEND 0
#define GROUP 1

//////////////总体四大界面 //////////////
#define PAGE_HOME_MSG 0
#define PAGE_HOME_CONTACT 1
#define PAGE_HOME_FILE 2
#define PAGE_HOME_CONFIG 3
#define PAGE_HOME_MORE 4

extern int lastDockIndex; //用于dock栏点击&恢复显示

////////////// 1.消息界面 //////////////
#define PAGE_HOME_MSG_DEFAULT 0 // 默认界界面
extern int currChatObjectID;

////////////// 2.联系人界面 //////////////
// 2.1联系人操作类型界面
#define PAGE_CONTACT_TYPE_FRIEND 0
#define PAGE_CONTACT_TYPE_GROUP 1

extern int contactType;//0-好友界面   1-群聊界面

// 2.2 联系人界面中的具体页面
#define PAGE_CONTACT_OP_DEFAULT 0 // 默认界界面 (公)
#define PAGE_CONTACT_OP_FRIEND_APPLY 1 //1 - 好友申请(好友)
#define PAGE_CONTACT_OP_ADD_CONTACT 2 //2 - 添加好友(好友)
#define PAGE_CONTACT_OP_CREATE_GROUP 3 //3 - 创建群 (群聊)
#define PAGE_CONTACT_OP_FRIEND_INFO 4 //4 - 用户信息 (好友)
#define PAGE_CONTACT_OP_GROUP_INFO 5 // 5 - 群信息 (群聊)

////////  3.文件管理  ////////
#define PAGE_FILE_ALL 0
#define PAGE_FILE_FRIEND 1
#define PAGE_FILE_GROUP 2

extern const int defaultPage;
extern int friendOpPage; // 1  2  4
extern int groupOpPage;// 3  5

extern ChatObjInfo *currChatObjInfo; //当前聊天对象

// 2.3联系人页面项对应的EC号
extern int currContactFrdID;
extern int currContactGrpID;

#define LINE_EDIT_START QString("QLineEdit{background-color: rgb(255, 255, 255);border-radius:5px;border:1px solid gray;padding-left:5px;}")
#define LINE_EDIT_FINISH QString("QLineEdit{background-color: rgb(245, 246, 247);border:none;padding-left: 5px;}")


#endif // UIRELY_H

