#ifndef UIHOME_H
#define UIHOME_H

#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopWidget>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QSharedPointer>
#include <QFileDialog>
#include <QTimer>
#include <QToolButton>
#include <QWidget>
#include <QRadioButton>
#include <QScrollBar>
#include <QDebug>

#include "global.h"
#include "device/file.h"
#include "db/database.h"
#include "item/itemchatobject.h"
#include "item/itemcontact.h"
#include "item/itemfriendapply.h"
#include "item/headpicture.h"
#include "ui/uichat.h"
#include "ui/uirely.h"

namespace Ui {
class UiHome;
}

class UiHome : public QWidget
{
    Q_OBJECT

public:
    explicit UiHome(QWidget *parent = nullptr);
    ~UiHome();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void userExit();
    void uiChatMax(const bool &state);
    void resourceRequest(const int &code, const int &frdId, const int &grpId,QJsonValue value); // 资源请求
    void sendChatMsg(const QString &msg, const int &frd, const int &grp, const QString& sendTime); // 发送消息
    void sendChatFile(const ChatObjInfo* chatObjInfo, const QStringList &filePathList, const int &frd, const int &grp);  // 发送文件

public slots:
    void on_setHeadPicture(const int index);
    void on_loginSuccess();
    void on_fileSendFinish(const ChatObjInfo* chatObjInfo,const QString &fileName,const QString sql);
    void on_connectState(bool state);
    void on_sendChatMsg(const QString &msg);
    void on_sendChatFile(const QStringList &filePathList);
    void on_resourceReply(const QByteArray &json);  // code资源请求回应处理--最后面

private slots:
    //Home
    void on_btn_headPic_clicked();
    void on_btn_message_clicked();
    void on_btn_contact_clicked();
    void on_btn_file_clicked();
    void on_btn_config_clicked();
    void on_btn_more_clicked();

    //Home-联系人类型
    void on_btn_friend_clicked();
    void on_btn_group_clicked();

    //Home-添加好友/群
    void on_btn_seek_clicked();
    void on_btn_add_clicked();
    void on_btn_addGroupPicture_clicked();
    void on_btn_sureAddGourp_clicked();
    void on_btn_cancelAddGroup_clicked();

    //Home-个人信息界面
    void on_btn_deleteContact_clicked();
    void on_btn_edit_clicked();
    void on_btn_editPicture_clicked();
    void on_btn_sendMsg_clicked();

    //Home-群聊信息界面
    void on_btn_quitGroup_clicked();
    void on_btn_grpEdit_clicked();
    void on_btn_grpEditMyName_clicked();
    void on_btn_editGrpPicture_clicked();
    void on_btn_grpSendMsg_clicked();

    //Home-文件管理界面
    void on_btn_openFilePos_clicked();
    void on_btn_openScreenShotPos_clicked();
    void on_btn_allFile_clicked();
    void on_btn_friendFile_clicked();
    void on_btn_groupFile_clicked();

    //Home-设置界面
    void on_btn_exitLogin_clicked();
    void on_btn_autoLogin_clicked();
    void on_btn_remainPwd_clicked();
    void on_btn_newNotifyVoice_clicked();
    void on_btn_groupNotifyVoice_clicked();
    void on_btn_selectNewPath_clicked();
    void on_btn_autoUpdate_clicked();
    void on_btn_surePasswordModify_clicked();
    void on_btn_resetPassword_clicked();

    void on_btn_submitSuggest_clicked();


private:
    void initLocalMsg(const ChatObjInfo *info,const int& chatObjId); //加载本地数据
    void initNeedRequest(); //请求资源
    void initFileList(); //初始化文件
    void initBaseContact();//联系人基础控件
    void createFileToolButton(const QString &fileName,const QString &text,const QString &filePath,const QString &tip);
    ChatObjInfo* createChatObject(const int &id,const int &type);
    void clearSeekInfo();
    void clearFriendInfo();
    void clearGroupInfo(bool isAll = true);
    void clearCreateGroupInfo();
    void clearFiles();
    void clearModifyPassword();
    void clearAllData();
    void closeHeadPicture();
    void deleteFriend(const int &friendId); //删除好友
    void quitGroup(const int&groupId);
    QString getHeadPicture(const int type,int picture=0);
    void newMessageTip(const int &type); //新消息提示
    bool messageHasImage(const QString &msg); //检测消失是否还有图片
    void setInputRegExp(); //正则表达式
    void dockIconChanged(const int &index); //dock栏按钮操作
    void buttonExclusive(ButtonType buttonType, void *item); //联系人按钮操作
    void connectChatObject(ItemChatObject* item,UiChat* uiChat);//聊天对象信号槽连接
    void contactCliked(ClikedType clikedType,ItemContact* item); //联系人按钮点击事件

private:
    QPoint offset;
    bool isDrag;
//    bool   isTop;  // 窗口置顶状态，默认不置顶
    QTimer *updateTimer;
    QMovie  *movie;
    int  createGroupPicture;

    QMap<int, ChatObjInfo *> chatObjects;
    QMap<int, ContactInfo *> friends;  // 0 - 3始终被占用
    QMap<int, ItemFriendApply*> applicants;
    QMap<int, ContactInfo *> groups; //0 始终被占用
    QMap<int, QToolButton*>  groupsMems; //群消息展示，群成员
    QMap<int, QRadioButton*> selectGrpMems;
    QMap<QString, QToolButton*>  files; //文件

    //按键状态
    QMap<int, QPushButton*> dock;
    bool editingSelfInfo;
    bool editingGrpInfo;
    bool editingMyGrpName;

    DataBase* db;
    File *file;
    HeadPicture *headPicture;
    bool selectingPicture;


private:
    Ui::UiHome *ui;
};

#endif  // UIHome_H
