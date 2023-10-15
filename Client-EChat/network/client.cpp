#include "network/client.h"

Client::Client(QObject *parent) : QObject(parent),
    isConnecting(false),
    tcpSocket(nullptr),
    timer(nullptr),
    dataLen(0)
{
    tcpSocket = new QTcpSocket(this);
//    connect(tcpSocket, &QTcpSocket::connected, this, &Client::on_connected);
//    connect(tcpSocket, &QTcpSocket::disconnected, this, &Client::on_disconnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &Client::on_readyRead);
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &Client::on_stateChanged);
//    connect(this,&Client::recvData,this,&Client::on_handleReply);

    // 设置定时器，实现断线重连
    timer = new QTimer(this);
    timer->setInterval(5000);
    connect(timer, &QTimer::timeout, this,[=]{
        tcpSocket->abort();
        tcpSocket->connectToHost(QHostAddress(SERVER_IP), SERVER_PORT);
    });

}

Client::~Client() { }

// 连接服务器
void Client::on_connectServer()
{
    if (isConnecting)
        return;
    tcpSocket->connectToHost(QHostAddress(SERVER_IP), SERVER_PORT);
}

// 断开连接
void Client::on_disconnectClient()
{
    if (QAbstractSocket::ConnectedState == tcpSocket->state())
        tcpSocket->disconnectFromHost();
}

void Client::on_newUserLogin()
{
    // 去更改服务器文件描述符
    if(lastId != 0){
        if(lastId == selfId)
            lastId = 0;
        else{
            QJsonObject updateObj;
            updateObj.insert("code",88);
            updateObj.insert("lastId",lastId);
//            updateObj.insert("userId",selfId);
            QJsonDocument doc(updateObj);
            send(doc.toJson(QJsonDocument::Compact)); //发送数据
        }
    }
}

// 101：验证用户
void Client::on_loginRequest(const int &userId, const QString &userPwd)
{
    QByteArray  md5 = QCryptographicHash::hash(userPwd.toUtf8(), QCryptographicHash::Md5);
    QJsonObject obj;
    obj.insert("code", 101);
    obj.insert("id", userId);
    obj.insert("password", QString(md5.toHex()));
    QJsonDocument doc(obj);
    send(doc.toJson(QJsonDocument::Compact));
}

// 102：注册用户
void Client::on_registerRequest(const QString &nickname, const QString &userPwd)
{
    QByteArray  md5 = QCryptographicHash::hash(userPwd.toUtf8(), QCryptographicHash::Md5);
    QJsonObject obj;
    obj.insert("code", 102);
    obj.insert("nickname", nickname);
    obj.insert("password", QString(md5.toHex()));
    QJsonDocument doc(obj);
    send(doc.toJson(QJsonDocument::Compact));
}

/***********************************************************************
 * @brief: code资源请求
 * @param: QJsonValue value
 * @note: code不为空；code后面的参数如果不使用需用0/NULL替代！
 ***********************************************************************/
void Client::on_resourceRequest(const int &code,const int &frdId, const int &grpId,QJsonValue value)
{
    QJsonObject obj;
    obj.insert("code", code);
    obj.insert("id", selfId);
    switch (code) {
        case 77:{
            obj.insert("recommend",value["recommend"]);
            break;
        }
        case 99:{
            break;
        }
        case 501:{
            obj.insert("applicant", frdId);
            obj.insert("agree",value["agree"]);
            break;
        }
        case 502: {
            obj.insert("applicant", frdId);
            obj.insert("groupId", grpId);
            obj.insert("agree",value["agree"]);
            break;
        }
        case 505: {
            if(frdId){
                obj.insert("type", 0);
                obj.insert("sideId", frdId);
            }else{
                obj.insert("type", 1);
                obj.insert("sideId", grpId);
            }
            break;
         }
        case 506: { //建群
            if(!value.isObject())  break;
            obj.insert("leaderId",value["leaderId"]);
            obj.insert("groupName",value["groupName"]);
            obj.insert("picture",value["picture"]);
            obj.insert("members",value["members"].toArray());
            obj.insert("memberCount",value["memberCount"].toInt());
            break;
        }
        case 601: {
             obj.insert("friendId", frdId);
            break;
        }
        case 602:
        case 603: {
            obj.insert("groupId", grpId);
            break;
        }
        case 604: {
            if(frdId)
                obj.insert("receiver", frdId);
            else
                obj.insert("receiver", grpId);
            break;
         }
        case 605: {
            if(frdId){
                obj.insert("type", 0);
                obj.insert("sideId", frdId);
            }else{
                obj.insert("type", 1);
                obj.insert("sideId", grpId);
            }
            break;
         }
        case 606:
        case 701:{ //修改用户信息
            obj.insert("nickname",value["nickname"]);
            obj.insert("selfSay",value["selfSay"]);
            obj.insert("sex",value["sex"]);
            obj.insert("phone",value["phone"]);
            obj.insert("region",value["region"]);
            obj.insert("email",value["email"]);
            break;
        }
        case 702:{
            break;
        }
        case 703:{
            int type = value["type"].toInt();
            obj.insert("groupId",grpId);
            obj.insert("type",type);
            if(type){
                obj.insert("nickname",value["nickname"]);
                obj.insert("intro",value["intro"]);
            }else
                obj.insert("userGroupNickname",value["userGroupNickname"]);
            break;
        }
        case 704:{ //修改密码
            QString newPwd = value["password"].toString();
            QByteArray  md5 = QCryptographicHash::hash(newPwd.toUtf8(), QCryptographicHash::Md5);
            obj.insert("password", QString(md5.toHex()));
            break;
        }
        case 705:{ //重置密码
            QString resetPwd = QString::number(selfId);
            QByteArray  md5 = QCryptographicHash::hash(resetPwd.toUtf8(), QCryptographicHash::Md5);
            obj.insert("password", QString(md5.toHex()));
            break;
        }
        case 706:{ //修改头像
            obj.insert("picture", value["picture"].toInt());
            obj.insert("type", value["type"].toInt());
            if(value["type"].toInt())
                obj.insert("groupId", value["groupId"].toInt());
            break;
        }
        case 801:{
            if(frdId){
                obj.remove("id");
                obj.insert("id", frdId);
            }
            break;
        }
        case 802:{
            break;
        }
        case 803:
        case 804:
        case 805: {
            obj.insert("groupId", grpId);
            break;
        }
        case 806:
        case 807:
        case 808: {
            obj.insert("friendId", frdId);
            break;
        }
        case 809: {
            break;
        }
        case 810:
        case 811:
        case 812: {
            break;
        }
        case 813: {
        obj.insert("friendId", frdId);
            break;
        }
        case 814:
        case 815:
        case 816: {
            break;
        }
        case 901: { //0-好友  1-群聊
            if(frdId){
                obj.insert("friendId", frdId);
                obj.insert("type",0);
                obj.insert("ps", value["ps"].toString());
            }
            else{
                obj.insert("type",1);
                obj.insert("groupId", grpId);
                obj.insert("ps", value["ps"].toString());
            }
            break;
        }
        case 902: {
            break;
        }
        case 903: {
            break;
        }
        default:
            break;
    }
    QJsonDocument doc(obj);
    send(doc.toJson(QJsonDocument::Compact));
}

// 发送消息
void Client::on_sendChatMsg(const QString &msg, const int &frd, const int &grp,const QString &sendTime)
{
    QJsonObject obj;

    obj.insert("code", 503);
    obj.insert("message", msg);
    obj.insert("sender", selfId);
    obj.insert("senderPicture", selfPicture);
    if(frd){
        obj.insert("receiver", frd);
        obj.insert("type", 0);
    }else {
        obj.insert("receiver", grp);
        obj.insert("type", 1);
    }
    obj.insert("sendTime", sendTime);
    obj.insert("msgType", MSG_TEXT);

    QJsonDocument doc(obj);
    send(doc.toJson(QJsonDocument::Compact));
}

// 多线程发送文件
void Client::on_sendChatFile(const ChatObjInfo* chatObjInfo,const QStringList &filePathList, const int &frd, const int &grp)
{
    //没网络情况
    if(!networkNormal)
        return;

    for (int i = 0; i < filePathList.count(); i++) {
        QFile file(filePathList.at(i)); //初始化文件

        //1.获取文件必要信息
        QFileInfo info(file);
        QString fileName = info.fileName();
        QString suffix = info.suffix();
//        qint64  fileSize = info.size();
//        qint64 sendSize = 0;

        //2.正式发送文件
        if (!file.open(QFile::ReadOnly)){
//            qDebug() << "file open failed...";
            emit fileSendFinish(chatObjInfo,"","");
            continue;
        }
        int type = 0;
        int receiver = 0;
        QJsonObject fileObj;
        fileObj.insert("code",103);
        fileObj.insert("sendState",false);
        fileObj.insert("fileName",fileName);
        fileObj.insert("sender",selfId);
        if(frd){
            receiver = frd;
        } else {
            type = 1;
            receiver = grp;
        }
        fileObj.insert("type",type);
        fileObj.insert("receiver",receiver);

        fileObj["data"];
        fileObj["dataLen"];
        while (!file.atEnd() && networkNormal) {
            QByteArray byteData;
            byteData = file.read(DATA_BOLCK_SIZE);
            QString strData(byteData);
            fileObj["data"] = strData;
            fileObj.insert("dataLen",byteData.length());

            QJsonDocument doc(fileObj);
            send(doc.toJson(QJsonDocument::Compact)); //发送数据
            QThread::msleep(200);
        }

        //3.发送完成，增加一些数据，要用存储数据库中！
//        fileObj.remove("data");
//        fileObj.remove("dataLen");
//        fileObj.insert("sendState",true);
//        fileObj.insert("sendTime",sendTime);
//        fileObj.insert("senderPicture",selfPicture);
//        fileObj.insert("fileName",fileName);
//        fileObj.insert("suffix",suffix);
////        fileObj.insert("fileSize",fileSize);
//        QJsonDocument doc(fileObj);
//        send(doc.toJson(QJsonDocument::Compact)); //发送数据
//        QThread::msleep(50);

        //4.资源关闭，
        file.close();

        //5.发送一个文件消息
        QString sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QJsonObject obj;
        obj.insert("code", 503);
        obj.insert("message", "");
        obj.insert("sender", selfId);
        obj.insert("receiver", receiver);
        obj.insert("type", 0);
        obj.insert("sendTime", sendTime);
        obj.insert("msgType", MSG_FILE);
        obj.insert("suffix",suffix);
        obj.insert("fileName",fileName);
        QJsonDocument msgDoc(obj);
        send(msgDoc.toJson(QJsonDocument::Compact));
        QThread::msleep(50);

        //5.发送完成，①通知聊天窗口文件发送成功；②存储至本地数据库中
        QString filePath = FILE_PATH(selfId);
        QString sql;
        if(type){
            sql = QString("INSERT INTO ec_messages(sender,grp_id,type,msg_type,send_time,file_name,suffix,file_path)  "
                          "VALUES(%1,%2,%3,'%4','%5','%6','%7','%8');"
                          ).arg(QString::number(selfId),QString::number(receiver),
                                QString::number(type),"file",sendTime,fileName,suffix,filePath);
        }else{
            sql = QString("INSERT INTO ec_messages(sender,frd_id,type,msg_type,send_time,file_name,suffix,file_path)  "
                          "VALUES(%1,%2,%3,'%4','%5','%6','%7','%8');"
                          ).arg(QString::number(selfId),QString::number(receiver),
                                QString::number(type),"file",sendTime,fileName,suffix,filePath);
        }
        emit fileSendFinish(chatObjInfo,fileName,sql);
    }
}


// 发送数据
void Client::send(const QByteArray &json)
{
    if(tcpSocket->state() != QAbstractSocket::ConnectedState)
        return;

    int dataLen = json.length();
    if (dataLen <= 0)
        return;

    // 封包
    int        ndataLen = qToBigEndian(dataLen + 1);  // 长度 = 数据长度 + 结束符
//    qDebug()<<"dataLen="<<dataLen;
    QByteArray packJson((char *)&ndataLen, PACK_HEAD_LENGTH);
    packJson.append(json);
    packJson.append('\0');  // 添加结束符

    // 发送数据
    tcpSocket->write(packJson);
    tcpSocket->waitForBytesWritten(); //等待发送完成

    QThread::usleep(200); //200
}

void Client::HandleReply(const QByteArray &json)
{ 
    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject   rootObj = doc.object();
    int           code = rootObj["code"].toInt();
    switch (code) {
         case 88:{
            if(rootObj["state"].toBool())
                lastId = 0; //清除上次用户id
           break;
        }
        case 99:{
            if(rootObj["state"].toBool()){
                networkNormal = true;
                isConnecting = true;
                emit connectState(isConnecting);
            }
            break;
        }
        case 101: { //登陆成功保存个人信息
//            if(rootObj["state"].toBool()){
//                if(!rootObj["loginState"].toBool()) {
//                    selfId = rootObj["id"].toInt();
//                    selfNickname = rootObj["nickname"].toString();
//                    selfPicture = rootObj["picture"].toInt();
//                }
//            }
            selfId = rootObj["id"].toInt();
            selfNickname = rootObj["nickname"].toString();
            selfPicture = rootObj["picture"].toInt();
            emit loginReply(json);
            break;
        }
        case 102: { //注册账户
            emit registerReply(json);
            break;
        }
        default:{
            emit resourceReply(json);
            break;
        }
    }
}

// 读取socket传入的数据
void Client::on_readyRead()
{
//    qDebug() << "client tid：" << QThread::currentThreadId();
    if (0 == tcpSocket->bytesAvailable())
        return;

    int        recvLen = 0;  // 已接收数据长度
    QByteArray json;         // 存放接收的json数据

    // 排除数据接收不完整情况，避免无效拆包
    if(0 == dataLen) {
        if (PACK_HEAD_LENGTH >= tcpSocket->bytesAvailable()) {
            qDebug() << "包头字节数 >= 可读字节数?";
            return;
        }

        // 拆包头
        QByteArray head = tcpSocket->read(PACK_HEAD_LENGTH);
        dataLen = qFromBigEndian(*(int *)head.data());  // 转成主机字节序
        //    qDebug() << "on_readyRead dataLen=" << dataLen;

        // 读数据
        while (recvLen < dataLen && tcpSocket->bytesAvailable()) {
            json.append(tcpSocket->read(dataLen - recvLen));
            recvLen = json.length();
        }
    }else{
        recvLen = buffer.size(); //先前接收的数据量
        // 接着上次的数据继续读取
        while (recvLen < dataLen && tcpSocket->bytesAvailable()) {
            buffer.append(tcpSocket->read(dataLen - recvLen));
            recvLen = buffer.length();
        }
        json = buffer;
    }
//    qDebug() << json.data();

    // 防止数据接收不完整情况
    if (recvLen != dataLen) {
//        qDebug() << "recvLen 数据接收不完整！";
        buffer = json;  //临时保存未读取完的数据
        return;
    }

    HandleReply(json); //处理回应的信息
    dataLen = 0;  //清空临时保存的数据
    buffer.clear();
}

// 成功连接至服务器
//void Client::on_connected() { }

// 与服务器断开连接
//void Client::on_disconnected { }

// socket状态改变
void Client::on_stateChanged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
//        case QAbstractSocket::HostLookupState:{
//            qDebug() << "连接状态: 主机查找中...";
//            break;
//        }
        case QAbstractSocket::ConnectingState:{
//            qDebug() << "连接状态: 正在连接中...";
            break;
        }
        case QAbstractSocket::UnconnectedState:{
            networkNormal = false;
            isConnecting = false;
//            qDebug() << "连接状态: 未连接状态！开始重连...";
            emit connectState(isConnecting);
            timer->start(); //开始断线重连
            break;
        }
        case QAbstractSocket::ConnectedState:{
//             qDebug()<< "连接状态: 已连接到服务器！"
//                     << "服务器信息: "
//                     << "IP=" << tcpSocket->peerAddress().toString() << " Port=" << QString::number(tcpSocket->peerPort());
            QJsonObject obj;
            obj.insert("code", 99);//确认连接代码
            obj.insert("id", selfId);//确认连接代码
            QJsonDocument doc(obj);
            QByteArray  connectMsg = doc.toJson(QJsonDocument::Compact);
            send(connectMsg); //确认是否已经连接
            timer->stop(); //停止断线重连
            break;
        }
        default:
            break;
    }
}
