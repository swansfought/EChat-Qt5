#include "database.h"

QMutex DataBase::mutex;
DataBase* DataBase::instance=nullptr;

DataBase* DataBase::getInstance()
{
    if (instance == nullptr) {
        QMutexLocker locker(&mutex);
        if (instance == nullptr) {
            instance = new DataBase;
        }
    }
    return instance;
}

bool DataBase::connectDB()
{
    if(isConnecting)
        return isConnecting;
    db = QSqlDatabase::addDatabase("QSQLITE"); //添加 SQL LITE数据库驱动
    QString path = CHATMSG_PATH(selfId);
    db.setDatabaseName(path + "/ChatMsg.db"); //设置数据库名称
    if(db.open()){
        isConnecting = true;

        QStringList tables = db.tables(QSql::Tables);
        if(!tables.contains("ec_messages")){
            QSqlQuery query;
            QString table_msg = "CREATE TABLE ec_messages( "
                                  "id INTEGER PRIMARY KEY AUTOINCREMENT , " // --COMMENT '自增id'
                                  "sender int NOT NULL , " //--COMMENT '发送者id，外键'
                                  "senderPicture int DEFAULT NULL , " //--COMMENT '发送者id，外键'
                                  "frd_id int DEFAULT NULL , " //--COMMENT '接收者id，外键'
                                  "grp_id int DEFAULT NULL , " //--COMMENT '接收者id，外键'
                                  "type tinyint(4) NOT NULL , " //--COMMENT '0-好友  1-群聊'
                                  "message longtext DEFAULT NULL, " //--COMMENT '消息'
                                  "msg_type varchar(50) NOT NULL, " //--COMMENT '消息类型'
                                  "send_time timestamp NOT NULL DEFAULT CURRENT_timestamp , " //--COMMENT '发送时间'
                                  "file_name varchar(255) DEFAULT NULL , " //--COMMENT '文件名'
                                  "suffix varchar(20) DEFAULT NULL , " //--COMMENT '文件后缀'
                                  "file_path text DEFAULT NULL); " ;  // --COMMENT '文件路径'
            query.exec(table_msg);
        }
        if(!tables.contains("ec_users")){
            QSqlQuery query;
            QString table_usr = "CREATE TABLE ec_users("
                                  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                  "usr_id int NOT NULL, "
                                  "usr_type tinyint(4) NOT NULL, " //--COMMENT '0-好友  1-群聊',
                                  "usr_nickname int DEFAULT NULL, "
                                  "usr_picture int DEFAULT NULL); ";
            query.exec(table_usr);
        }
        qDebug()<<"数据库连接成功...";
        return isConnecting;
    }
    qDebug()<<"数据库已是连接状态...";
    return false;
}

void DataBase::disconnectDB()
{
    if(isConnecting){
        db.close();
        qDebug()<<"数据库已断开...";
    }
}

bool DataBase::addLocalMsg(const QString &msg, const int &sender, const int &senderPicture,
                           const int &receiver, const int &groupId, const int &type, const QString &sendTime)
{
    QString sql;
    QSqlQuery query;
    if(!type){ //好友
        sql = QString("INSERT INTO ec_messages(sender,senderPicture,frd_id,type,message,msg_type,send_time) "
                      "VALUES(%1, %2, %3, %4,'%5','text','%6');"
                      ).arg(QString::number(sender),QString::number(senderPicture),
                            QString::number(receiver),QString::number(type), msg, sendTime);
    }else{
        sql = QString("INSERT INTO ec_messages(sender,senderPicture,grp_id,type,message,msg_type,send_time) "
                      "VALUES(%1, %2, %3, %4,'%5','text','%6');"
                      ).arg(QString::number(sender),QString::number(senderPicture),
                            QString::number(groupId),QString::number(type), msg, sendTime);
    }
//    qDebug()<<sql;

    if(query.exec(sql))
        return true;
    qDebug()<<"本地text数据添加失败...";
    return false;
}

void DataBase::addLocalUser(const int &type, const int &id, const QString &nickname, const int &picture)
{
    QString sql;
    QSqlQuery query;

    //判断是否存在该用户，存在则为更新操作
    sql = QString("SELECT usr_id FROM ec_users WHERE usr_id=%1;").arg(QString::number(id));
    if(query.exec(sql) && query.next()){
        query.prepare("UPDATE ec_users SET usr_nickname=?,usr_picture=?  WHERE usr_id=?;");
        query.bindValue(0,nickname);
        query.bindValue(1,picture);
        query.bindValue(2,id);
        query.exec();
    }else{
        query.prepare("INSERT INTO ec_users(usr_id,usr_type,usr_nickname,usr_picture) "
                      "VALUES(?,?,?,?);");
        query.bindValue(0,id);
        query.bindValue(1,type);
        query.bindValue(2,nickname);
        query.bindValue(3,picture);
        query.exec();
    }
//    if(!query.exec()){
//        if(type)
//            qDebug()<<"群聊["<<id<<"]信息添加失败...";
//        else
//            qDebug()<<"好友["<<id<<"]信息添加失败...";
    //    }
}

void DataBase::updateUserInfo(const int &id, const int&type, const QString &nickname, const int &picture)
{
    QString sql;
    QSqlQuery query;

    //判断是否存在该用户，存在才去更新
    sql = QString("SELECT usr_id FROM ec_users WHERE usr_id=%1;").arg(QString::number(id));
    if(query.exec(sql) && query.next()){
        query.prepare("UPDATE ec_users SET usr_nickname=?,usr_picture=?  WHERE usr_id=?;");
        query.bindValue(0,nickname);
        query.bindValue(1,picture);
        query.bindValue(2,id);
        if(!query.exec())
            qDebug()<<"好友/群里信息更新失败...";
    }else{
        // 新用户和群聊
        addLocalUser(type,id,nickname,picture);
    }
}

DataBase::DataBase(QObject *parent)
    : QObject{parent},isConnecting(false)
{

}
