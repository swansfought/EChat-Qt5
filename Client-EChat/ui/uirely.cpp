
#include "uirely.h"

int lastDockIndex; //用于dock栏点击&恢复显示，默认消息被点击

int currChatObjectID;

int contactType;//0-好友界面(默认)   1-群聊界面

const int defaultPage = 0;
int friendOpPage; // 1  2  4  默认第0页
int groupOpPage;// 3  5    默认第0页

ChatObjInfo *currChatObjInfo = nullptr; //当前聊天对象

// 2.3联系人页面项对应的EC号
int currContactFrdID;
int currContactGrpID;


