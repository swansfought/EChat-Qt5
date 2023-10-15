 #ifndef UICHAT_H
#define UICHAT_H

#include <QCoreApplication>
#include <QDebug>
#include <QToolButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QWidget>
#include <QFontMetrics>
#include <QApplication>
#include <QClipboard>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QApplication>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QClipboard>
#include <QMimeData>
#include <QBuffer>
#include <QMovie>
#include <QMimeData>
#include <QMessageBox>

#include "db/database.h"
#include "device/screenshot.h"
#include "emo/emoji.h"

#define UICHAT_MAX_SIZE QSize(1670, 962)  // 最大窗口
#define UICHAT_MIN_SIZE QSize(529, 473)   // 最小窗口
#define MAX_CONTENT_SHOW_HEIGHT 585
#define MIN_CONTENT_SHOW_HEIGHT 255

#define MSG_MARGIN_LEFT 150
#define MSG_MARGIN_RIGHT 45
#define MSG_DURATION_TIP 5

#define IMAGE_WIDTH 300.0

#define MSG_TEXT "text"
#define MSG_FILE "file"
#define MSG_IMAGE "image"

namespace Ui {
class UiChat;
}

class UiChat : public QWidget
{
    Q_OBJECT

public:
    explicit UiChat(QWidget *parent = nullptr);
    ~UiChat();

    void setSidebarVisible(const bool &visible);
    void setSideNickname(const QString &nickname,const int& type);
    void setSideType(const int &type);
    void setInputFocus();

    void setMemberType(const int&adminCount,const int &memberCount);
    void setGroupTopText(const QString &text);
    void addGroupMembers(const int &index, const QString &text,const QString &tip);
    void addFileMsg(const QString& fileName,const int& uerId);

    //文件发送提示
    void setFileNumTip(const int&count);
    void removeFileNumTip();

    //离线消息加载
    void recvChatMsg(const int &type, const QJsonObject &msgObject);
    //本地数据加载
    void localChatMsg(const int &type,const QJsonObject &msgObject);
    //好友/群聊之间的数据接受
    void recvChatMsg(const QJsonObject &singleObject);

    void closeEmojiShow();

public:
//    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void sendChatMsg(const QString &msg);
    void sendChatFile(QStringList &filePathList);

public slots:
    void on_uiChatMax(const bool &state);

private slots:
    void on_btn_send_clicked();
    void on_btn_emotion_clicked();
    void on_btn_image_clicked();
    void on_btn_file_clicked();
    void on_btn_screenshot_clicked();
    void on_btn_more_clicked();
    void on_btn_hideSlide_clicked();

private:
    QString getHeadPicture(const int type,int picture=0);

    void recvTimeTip(const QString &sendTime); //接受消息时间提示
    void sendTimeTip(); //发送消息时间提示
    void sendFileLoading(const bool&state); //发送文件加载提示

    //数据插入
    void insertHtml(const QString &html);
    void showSelfMsg(const int &type,const QString&nickname,const int &index,const QString &msg);
    void showSideMsg(const int &type,const QString&nickname,const int &index,const QString &msg);

    //消息图片提取
    QList<QString> getImageFromHtml(const QString& html);
    void saveImageFromMsg(const QString& msg);

    bool messageHasImage(const QString &msg); //检测消失是否还有图片

private:
    int toolIndex;//用来提供插入
    QFont font;
    bool hideState;

    int sideType;//对方类型，好友/群聊

    QMovie *movie;
    Emoji *emoji;
    bool  emojiShow;
    ScreenShot *screenshot; //截屏

    QDateTime lastRecvDateTime;
    QDateTime lastSendDateTime;

private:
    Ui::UiChat *ui;
};

#endif  // UICHAT_H
