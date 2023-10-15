#include "global.h"

int selfId;  // 当前用户id
int lastId; //上次用户的id
QString selfNickname;
int selfPicture;
QString defaultPicture = ":/img/defaultPicture.ico";

bool networkNormal = false; //网络是否正常，只有与服务器保持连接才为true

bool modifyPassword = false;

int msgCounts;

#include <QRegExp>
#include <QRegExpValidator>



