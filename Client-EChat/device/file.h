#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QCoreApplication>
#include <QSettings>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QDesktopServices>
#include <QProcess>

#include "db/database.h"


class File : public QObject
{
    Q_OBJECT
public:
    static File* getInstance();

    void checkDataFolder();
    void checkConfigFile();

    void updateAutoLogin(const bool& state);
    void updateRemainPwd(const bool& state);
    void updateNewNotifyVoice(const bool& state);
    void updateGroupNotifyVoice(const bool& state);
    void updateAutoUpdate(const bool& state);
    void updateFilePath(const QString& filePath);

    void updateEchatId(const int& userId);
    void updatePassword(const QString& password);

    bool getAutoLogin(){ return AutoLogin; };
    bool getRemainPwd(){ return RemainPwd; };
    bool getNewNotifyVoice(){ return NewNotifyVoice; };
    bool getGroupNotifyVoice(){ return GroupNotifyVoice; };
    bool getAutoUpdate(){ return AutoUpdate; };
    QString getFilePath(){ return FilePath; };
    int getEchatId(){ return echatId; };
    QString getPassword(){ return encryptPassword(password); };

private:
    QString encryptPassword(const QString& password);

private:
     bool AutoLogin;
     bool RemainPwd;
     bool NewNotifyVoice;
     bool GroupNotifyVoice;
     bool AutoUpdate;
     QString FilePath;
     int  echatId; //默认0
     QString password; //默认""

    explicit File(QObject *parent = nullptr);
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    static QMutex   mutex;
    static File* instance;
};

#endif // FILE_H
