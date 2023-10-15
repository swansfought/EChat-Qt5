#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QSqlDatabase> //数据库
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

#include "global.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    static DataBase* getInstance();

    bool connectDB();
    void disconnectDB();

    bool addLocalMsg(const QString &msg, const int &sender, const int &senderPicture,
                     const int &receiver, const int &groupId,const int &type,const QString &sendTime);
    bool addLocalFileMsg();

    void addLocalUser(const int&type, const int&id, const QString&nickname, const int &picture);

    void updateUserInfo(const int&id,const int&type, const QString&nickname, const int &picture);

signals:

private:
    bool isConnecting;
    QSqlDatabase db;//数据库

private:
    explicit DataBase(QObject *parent = nullptr);
    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    static QMutex mutex;
    static DataBase* instance;
};

#endif // DATABASE_H
