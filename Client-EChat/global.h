#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QCoreApplication>

//个人的基本信息，非常重要，多次被其他地方使用！！！！
extern int selfId;
extern int lastId; // 是一个用户遗留的id
extern QString selfNickname;
extern int selfPicture;

//extern bool userExiting;
extern bool networkNormal; // 网络情况

extern bool modifyPassword; //修改密码，用来表示退出情况是修改密码所致

extern int msgCounts; //消息数量

#define MSG_COUNT 80

#define MSG_TEXT "text"
#define MSG_IMAGE "image"
#define MSG_FILE "file"

#define DATA_FOLDER "EChat Data"
#define USER_FOLDER(userId) QString::number(userId)
#define AUDIO_FOLDER "Audio"
#define CHATMSG_FOLDER "ChatMsg"
#define FILE_FOLDER "File"
#define IMAGE_FOLDER "Image"
#define IMAGE_HEADPICTURE_FOLDER "HeadPicture"
#define SCREENSHOT_FOLDER "ScreenShot"
#define VIDEO_FOLDER "Video"

#define CURR_PATH QCoreApplication::applicationDirPath()
#define CONFIG_FILE CURR_PATH + "/config.ini"
#define DATA_PATH CURR_PATH+"/"+DATA_FOLDER
#define USER_PATH(userId) DATA_PATH+"/"+USER_FOLDER(userId)

#define AUDIO_PATH(userId) USER_PATH(userId)+"/"+AUDIO_FOLDER
#define CHATMSG_PATH(userId) USER_PATH(userId)+"/"+CHATMSG_FOLDER
#define FILE_PATH(userId) USER_PATH(userId)+"/"+FILE_FOLDER

#define IMAGE_PATH(userId) USER_PATH(userId)+"/"+IMAGE_FOLDER
#define IMAGE_HEADPICTURE_PATH(userId) IMAGE_PATH(userId)+"/"+IMAGE_HEADPICTURE_FOLDER

#define SCREENSHOT_PATH(userId) USER_PATH(userId)+"/"+SCREENSHOT_FOLDER
#define VIDEO_PATH(userId) USER_PATH(userId)+"/"+VIDEO_FOLDER

// ItemFriend
#define SELECT_BACKGROUND "#widget_background{background-color:rgb(225, 225, 225);}"
#define ENTER_BACKGROUND "#widget_background{background-color:rgb(235, 235, 235);}"
#define LEAVE_BACKGROUND "#widget_background{background-color:rgb(240, 240, 240);}"

// ItemChatObject
#define ONLINE_STATE "#lab_onlineState{background-color:rgb(30, 215, 109);border-radius:6px;}"
#define OFFLINE_STATE "#lab_onlineState{background-color:rgb(135,135,135);border-radius:6px;}"

#define NUM_REG "^[0-9]+$"
#define PWD_REG "^[A-Za-z0-9]+$"
#define ZH_REG "^[\u4e00-\u9fa5]{0,}$"
#define ZH_EN_NUM_REG "^[A-Za-z\u4E00-\u9FA50-9]+$"
#define EMAIL_REG "^[a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+$"
//#define PHONE_REG "/^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$/" //??有问题



#endif // GLOBAL_H
