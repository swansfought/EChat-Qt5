#ifndef CLIENT_H
#define CLIENT_H

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMutex>
#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QBuffer>
#include <QtEndian>

#include "global.h"
#include "thumbnail.h"

#define SERVER_IP "81.68.113.233"  // 服务器IP
#define SERVER_PORT 7799           // 默认端口
#define PACK_HEAD_LENGTH 4
#define DATA_BOLCK_SIZE 1024 // 每个块的大小

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);
    ~Client();

    bool connectStatus() { return isConnecting; }

signals:
    void connectState(bool bol);
    void fileSendFinish(const ChatObjInfo* info,const QString &fileName,const QString sql);

    // --服务器回应--
    void loginReply(QByteArray json);
    void registerReply(QByteArray json);
    void resourceReply(QByteArray json);

public slots:
//    void on_connected();
//    void on_disconnected();
    void on_stateChanged(QAbstractSocket::SocketState socketState);
    void on_readyRead();

    void on_connectServer();
    void on_disconnectClient();

    // --处理客户端请求--
    void on_newUserLogin();
    void on_loginRequest(const int &userId, const QString &userPwd);                                  // 101：验证用户
    void on_registerRequest(const QString &nickname, const QString &userPwd);                         // 102：注册用户
    void on_resourceRequest(const int &code, const int &frdId, const int &grpId,QJsonValue value);    // code：资源请求

    void on_sendChatMsg(const QString &msg, const int &frd, const int &grp, const QString& sendTime);
    void on_sendChatFile(const ChatObjInfo* chatObjInfo,const QStringList &filePathList, const int &frd, const int &grp);
//    void on_sendChatImage(const QStringList &filePathList, const int &selfId, const int &frd, const int &grp);

private:
    void send(const QByteArray &json);
    void HandleReply(const QByteArray &json);

private:
    bool         isConnecting;
    QTcpSocket  *tcpSocket;
    QTimer      *timer;         //
    QByteArray   buffer;
    int          dataLen;

};

#endif  // CLIENT_H
