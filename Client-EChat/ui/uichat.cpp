#include "ui/uichat.h"

#include "ui_uichat.h"

UiChat::UiChat(QWidget *parent) :
    QWidget(parent),
    toolIndex(0),
    hideState(false),
    sideType(0),
    emoji(nullptr),
    emojiShow(false),
    screenshot(nullptr),
    ui(new Ui::UiChat)
{
    ui->setupUi(this);

    // 加载样式表
    QFile file(":/qss/chat.qss");
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);
    QString     styleSheet = file.readAll();
    this->setStyleSheet(styleSheet);
    file.close();

    //发送文件加载图
    movie = new QMovie(":/img/threeLoading.gif"); //lineLoading.gif threeLoading.gif
    ui->lab_loading->setMovie(movie);
    ui->widget_loading->setVisible(false);

    // 群侧边隐藏按钮
    font.setFamily("微软雅黑");
    font.setPointSize(7);
    ui->widget_info->setVisible(false);
    ui->btn_hideSlide->setToolTip("展开");
    ui->btn_hideSlide->setIcon(QIcon(":/img/arrow-left.png"));

    ui->contentShow->installEventFilter(this);
    ui->contentEdit->installEventFilter(this);
    ui->widget_sidebar->installEventFilter(this);
    ui->line_split->installEventFilter(this);

//    ui->contentEdit->setAcceptDrops(false);
//    ui->contentEdit->setAcceptRichText(false);

    //表情包选择栏
    emoji = new Emoji(this);
    emoji->move(QPoint(0,125)); //最小化时的显示位置
    emoji->hide();
    connect(emoji,&Emoji::emo,this,[=](QString text){
        emoji->hide();
        emojiShow = false;
        ui->contentEdit->insertPlainText(text);
        ui->contentEdit->setFocus();
    });

    // 截屏
    screenshot = new ScreenShot(this);
    screenshot->showFullScreen();// 全屏显示
    screenshot->hide();
    connect(screenshot,&ScreenShot::screenImage,this,[=](QPixmap pixmap){
        if(pixmap.isNull())
            return;

        // 1.获取截图图片，并且缩放到合适比例
        int w = pixmap.width();
        int h = pixmap.height();
        if(w > IMAGE_WIDTH){
            qreal ratio = w/IMAGE_WIDTH;
            w /= ratio;
            h /= ratio;
        }

        // 2.QPixmap转换为Base64编码的二进制数据
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        buffer.close();

        //3.将Base64编码的二进制数据转换为QString
        QString base64 = QString::fromLatin1(byteArray.toBase64().data());

        //4.设置QTextImageFormat属性
        QTextImageFormat imageFormat;
        imageFormat.setName("data:image/png;base64," + base64);
        imageFormat.setProperty(QTextFormat::UserProperty, "data:image/png;base64," + base64); //保存原始图像的Base64编码

        //5.将图片插入 QTextEdit
        ui->contentEdit->insertHtml(QString("<img  width=\"%1\" height=\"%2\"  "
                                            "src=\"data:image/png;base64, %3 \"/>").arg(QString::number(w),
                                                                                        QString::number(h),
                                                                                        base64));
    });
}

UiChat::~UiChat()
{
    delete ui;
//    qDebug() << "~UiChat";
}

void UiChat::setSidebarVisible(const bool &visible) { ui->widget_sidebar->setVisible(visible); }

void UiChat::setSideNickname(const QString &nickname,const int& type)
{
    ui->lab_sideNickname->setText(nickname);
    if(type)
        ui->lab_sideTitle->setText("[群聊]");
    else
        ui->lab_sideTitle->setText("[好友]");
}

void UiChat::setSideType(const int &type)
{
    if(type < 0 || type > 1)
        return;
    sideType = type;
}

void UiChat::setInputFocus() { ui->contentEdit->setFocus(); }

void UiChat::setMemberType(const int&adminCount,const int &memberCount){
    ui->lab_memberType->setText(QString("%1 / %2").arg(QString::number(adminCount),
                                                      QString::number(memberCount)));
}

void UiChat::setGroupTopText(const QString &text) { ui->plain_topText->appendPlainText(text); }

void UiChat::addGroupMembers(const int &index, const QString &text,const QString &tip)
{

    QToolButton *toolBtn = new QToolButton(this);
    toolBtn->setMinimumSize(QSize(60,26));
    toolBtn->setMaximumSize(QSize(60,26));
    toolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBtn->setFont(font);
    toolBtn->setText(text);
    toolBtn->setToolTip(tip);

    QString picture;
    if (index <= 0)
        picture = ":/img/head/person.png"; //默认头像
    else
        picture = QString(":/img/head/%1.png").arg(QString::number(index));
    toolBtn->setIcon(QIcon(picture));
    toolBtn->setIconSize(QSize(24,24));

    ui->vLayout_members->insertWidget(toolIndex++, toolBtn);
}

void UiChat::addFileMsg(const QString& fileName,const int& uerId)
{
    if(fileName.isEmpty())
        return;
    QString message = "<img width=\"36\" height=\"36\" src=\":/img/file.png\">" + fileName;

    if(uerId == selfId)
        showSelfMsg(0,selfNickname,selfPicture,message);
    else{
         // 获取该用户名称和头像
        QString sql = QString("SELECT * FROM ec_users WHERE usr_id=%1;").arg(QString::number(uerId));
        QSqlQuery query;
        if(query.exec(sql) && query.next())
            showSideMsg(0,query.value("nickname").toString(),query.value("picture").toInt(),message);
    }
}

void UiChat::setFileNumTip(const int&count)
{
    if(0 == count){
        ui->lab_loadingTip->clear();
        sendFileLoading(false);
    }else{
        ui->lab_loadingTip->setText(QString("0/%1").arg(QString::number(count)));
        ui->lab_loadingTip->setToolTip(QString("剩0/总%1").arg(QString::number(count)));
    }
}

void UiChat::removeFileNumTip()
{
    QString text = ui->lab_loadingTip->text();
    QStringList list = text.split("/");
    int sendNum = list.at(0).toInt() + 1;
    //文件发送完成
    if(list.at(1).toInt() == sendNum)
        sendFileLoading(false);
    else{
        ui->lab_loadingTip->setText( QString("%1/%2").arg(QString::number(sendNum), list.at(1)) );
        ui->lab_loadingTip->setToolTip(QString("剩%1/总%2").arg(QString::number(sendNum), list.at(1)) );
    }
}

void UiChat::recvChatMsg(const int &type,const QJsonObject &msgObject)
{
    QJsonArray chatMsgList = msgObject["chatMsgList"].toArray();
    int counts = chatMsgList.count();
    //群
    if(type){
        int groupId = msgObject["groupId"].toInt();
        for (int i = 0; i < counts; ++i) {
            QJsonObject subObj = chatMsgList[i].toObject();
            QString message = subObj["message"].toString();
            // 保存图片，如果有的话
            saveImageFromMsg(message);

            QString msgType = subObj["msgType"].toString();
            QString sendTime = subObj["sendTime"].toString();
            recvTimeTip(sendTime); //消息时间提示
            int senderId = subObj["sender"].toInt();

            QJsonObject senderInfo = subObj["senderInfo"].toObject();
            QString nickname = senderInfo["nickname"].toString();
            int picture = senderInfo["picture"].toInt();

            //文件消息
            QString fileName="",suffix="",filePath="";
            if("text" != msgType){
                fileName = subObj["fileName"].toString();
                suffix = subObj["suffix"].toString();
                filePath = subObj["filePath"].toString();
                message = "<img width=\"36\" height=\"36\" src=\":/img/file.png\">" + fileName;
            }

            if(senderId == selfId)
                showSelfMsg(type,nickname,picture,message);
            else
                showSideMsg(type,nickname,picture,message);

            //存储至本地数据库中
            QSqlQuery query;
            query.prepare("INSERT INTO ec_messages(sender,senderPicture,grp_id,type,message,"
                          "msg_type,send_time,file_name,suffix,file_path)  "
                          "VALUES(?,?,?,?,?,?,?,?,?,?);");
            query.bindValue(0,senderId);
            query.bindValue(1,picture);
            query.bindValue(2,groupId);
            query.bindValue(3,type);
            query.bindValue(4,message);
            query.bindValue(5,msgType);
            query.bindValue(6,sendTime);
            query.bindValue(7,fileName);
            query.bindValue(8,suffix);
            query.bindValue(9,filePath);
            if(!query.exec())
                qDebug()<<"本地数据存储失败...";
        }
    }
    else{
        //拿到好友头像
        int picture = msgObject["picture"].toInt();
        int friendId = msgObject["friendId"].toInt(); // 好友ID

        //拿到聊天消息
        for (int i = 0; i < counts; ++i) {
            QJsonObject subObj = chatMsgList[i].toObject();
            QString message = subObj["message"].toString();
            // 保存图片，如果有的话
//            saveImageFromMsg(message);

            QString msgType = subObj["msgType"].toString();
            QString sendTime = subObj["sendTime"].toString();
            recvTimeTip(sendTime); //消息时间提示
            int senderId = subObj["sender"].toInt();

            //文件消息
            QString fileName="",suffix="",filePath="";
            if("text" != msgType){
                fileName = subObj["fileName"].toString();
                suffix = subObj["suffix"].toString();
                filePath = subObj["filePath"].toString();
                message = "<img width=\"36\" height=\"36\" src=\":/img/file.png\">" + fileName;
            }

            if(senderId == selfId)
                showSelfMsg(type,"",picture,message);
            else
                showSideMsg(type,"",picture,message);

            //存储至本地数据库中
            QSqlQuery query;
            query.prepare("INSERT INTO ec_messages(sender,senderPicture,frd_id,type,message,"
                          "msg_type,send_time,file_name,suffix,file_path)  "
                          "VALUES(?,?,?,?,?,?,?,?,?,?);");
            //区分接收者和发送者
            if(senderId == selfId){
                query.bindValue(0,senderId);
                query.bindValue(1,picture);
                query.bindValue(2,friendId);
            }else{
                query.bindValue(0,senderId);
                query.bindValue(1,picture);
                query.bindValue(2,selfId);
            }
            query.bindValue(3,type);
            query.bindValue(4,message);
            query.bindValue(5,msgType);
            query.bindValue(6,sendTime);
            query.bindValue(7,fileName);
            query.bindValue(8,suffix);
            query.bindValue(9,filePath);
            if(!query.exec())
                qDebug()<<"本地数据存储失败...";
        }
    }
}

void UiChat::recvChatMsg(const QJsonObject &singleObject)
{
    //拿到对方发送的数据信息
    int sender = singleObject["sender"].toInt();
    QJsonObject senderInfo = singleObject["senderInfos"].toObject();
    QString nickname = senderInfo["nickname"].toString();
    int picture = senderInfo["picture"].toInt();

    int receiver = singleObject["receiver"].toInt();
    int type = singleObject["type"].toInt();
    QString msgType = singleObject["msgType"].toString();
    QString message = singleObject["message"].toString();
    // 保存图片，如果有的话
    saveImageFromMsg(message);

    QString sendTime = singleObject["sendTime"].toString();
    recvTimeTip(sendTime); //消息时间提示

    //文件消息
    QString fileName="",suffix="",filePath="";
    if("text" != msgType){
        picture = senderInfo["picture"].toInt();
        fileName = singleObject["fileName"].toString();
        suffix = singleObject["suffix"].toString();
        filePath = FILE_PATH(selfId);
        message = "<img width=\"36\" height=\"36\" src=\":/img/file.png\">" + fileName;
    }

    if(type){  //群
        if(sender == selfId)
            showSelfMsg(type,nickname,picture,message);
        else
            showSideMsg(type,nickname,picture,message);

        //存储至本地数据库中
        QSqlQuery query;
        query.prepare("INSERT INTO ec_messages(sender,senderPicture,grp_id,type,message,"
                      "msg_type,send_time,file_name,suffix,file_path)  "
                      "VALUES(?,?,?,?,?,?,?,?,?,?);");
        query.bindValue(0,sender);
        query.bindValue(1,picture);
        query.bindValue(2,receiver);
        query.bindValue(3,type);
        query.bindValue(4,message);
        query.bindValue(5,msgType);
        query.bindValue(6,sendTime);
        query.bindValue(7,fileName);
        query.bindValue(8,suffix);
        query.bindValue(9,filePath);
        if(!query.exec())
            qDebug()<<"本地数据存储失败...";
    }
    else{  
        if(sender == selfId)
            showSelfMsg(type,"",picture,message);
        else
            showSideMsg(type,"",picture,message);

        //存储至本地数据库中
        QSqlQuery query;
        query.prepare("INSERT INTO ec_messages(sender,senderPicture,frd_id,type,message,"
                      "msg_type,send_time,file_name,suffix,file_path)  "
                      "VALUES(?,?,?,?,?,?,?,?,?,?);");
        query.bindValue(0,sender);
        query.bindValue(1,picture);
        query.bindValue(2,receiver);
        query.bindValue(3,type);
        query.bindValue(4,message);
        query.bindValue(5,msgType);
        query.bindValue(6,sendTime);
        query.bindValue(7,fileName);
        query.bindValue(8,suffix);
        query.bindValue(9,filePath);
        if(!query.exec())
            qDebug()<<"本地数据存储失败...";
    }
}

void UiChat::localChatMsg(const int &type, const QJsonObject &msgObject)
{
    QString message = msgObject["message"].toString();
    QString msgType = msgObject["msgType"].toString();
    QString sendTime = msgObject["sendTime"].toString();
    recvTimeTip(sendTime); //消息时间提示
//    qDebug()<<msgObject;

    //文件消息
    if("text" != msgType)
        message = "<img width=\"36\" height=\"36\" src=\":/img/file.png\">" + msgObject["fileName"].toString();

    //群
    if(type){
        int senderId = msgObject["sender"].toInt();

        QJsonObject senderInfo = msgObject["senderInfo"].toObject();
        QString nickname = senderInfo["nickname"].toString();
        int picture = senderInfo["picture"].toInt();

        if(senderId == selfId){
            showSelfMsg(type,nickname,picture,message);
            lastSendDateTime = QDateTime::fromString(sendTime,"yyyy-MM-dd hh:mm:ss"); //保存上次的发送时间
        }else{
            showSideMsg(type,nickname,picture,message);
            lastRecvDateTime = QDateTime::fromString(sendTime,"yyyy-MM-dd hh:mm:ss"); //保存上次的接收时间
        }

    }else{
        int picture = msgObject["picture"].toInt();
        int senderId = msgObject["sender"].toInt();

        if(senderId == selfId){
            showSelfMsg(type,"",picture,message);
            lastSendDateTime = QDateTime::fromString(sendTime,"yyyy-MM-dd hh:mm:ss"); //保存上次的发送时间
        } else{
            showSideMsg(type,"",picture,message);
            lastRecvDateTime = QDateTime::fromString(sendTime,"yyyy-MM-dd hh:mm:ss"); //保存上次的接收时间
        }
    }
}

void UiChat::closeEmojiShow()
{
    // 在显示就给关闭
    if(emojiShow){
        emoji->hide();
        emojiShow = !emojiShow;
    }
}

bool UiChat::eventFilter(QObject *watched, QEvent *event)
{
    // 关闭表情选择框
    if(event->type() == QEvent::FocusIn){
        if(dynamic_cast<QWidget*>(watched) == ui->contentShow || dynamic_cast<QWidget*>(watched) == ui->contentEdit){
            closeEmojiShow();
        }
    }

    // 快捷键处理
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::Shortcut){
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(dynamic_cast<QWidget*>(watched) == ui->contentShow){
            //忽略全选
            if(keyEvent->key()==Qt::Key_A && keyEvent->modifiers()==Qt::ControlModifier){
                return true;
            }
        }
        else if(dynamic_cast<QWidget*>(watched) == ui->contentEdit){
            // 回车发送，shift+enter换行
            if(keyEvent->key() == Qt::Key_Return && keyEvent->modifiers()!=Qt::SHIFT){
                on_btn_send_clicked();
                return true;
            }
             //ctrl+v粘贴图片
            else if(keyEvent->key()==Qt::Key_V && keyEvent->modifiers()==Qt::ControlModifier){
                    // 获取剪切板数据
                    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
                    if (mimeData->hasImage()) {
                        // 1.从剪切板获取图片，并且缩放到合适比例
                        QImage image = qvariant_cast<QImage>(mimeData->imageData());
                        int w = image.width();
                        int h = image.height();
                        if(w > IMAGE_WIDTH){
                            qreal ratio = w/IMAGE_WIDTH;
                            w /= ratio;
                            h /= ratio;
                        }

                        // 2.QImage->QPixmap然后换为Base64编码的二进制数据
                        QPixmap pixmap = QPixmap::fromImage(image);
                        QByteArray byteArray;
                        QBuffer buffer(&byteArray);
                        buffer.open(QIODevice::WriteOnly);
                        pixmap.save(&buffer, "PNG");
                        buffer.close();

                        //3.将Base64编码的二进制数据转换为QString
                        QString base64 = QString::fromLatin1(byteArray.toBase64().data());

                        //4.设置QTextImageFormat属性
                        QTextImageFormat imageFormat;
                        imageFormat.setName("data:image/png;base64," + base64);
                        imageFormat.setProperty(QTextFormat::UserProperty, "data:image/png;base64," + base64); //保存原始图像的Base64编码

                        //5.将图片插入 QTextEdit
                        ui->contentEdit->insertHtml(QString("<img  width=\"%1\" height=\"%2\"  "
                                                            "src=\"data:image/png;base64, %3 \"/>").arg(QString::number(w),
                                                                                                        QString::number(h),
                                                                                                        base64));
//                        showSelfMsg(sideType,selfNickname,selfPicture,"<img src='data:image/png;base64," + base64Image + "' />");
                    }
                }
        }
    }
    return QWidget::eventFilter(watched,event);
}

void UiChat::on_uiChatMax(const bool &state)
{
    if (state) {
        this->setMaximumSize(UICHAT_MAX_SIZE);
        this->setMinimumSize(UICHAT_MAX_SIZE);
        ui->contentShow->setMinimumHeight(MAX_CONTENT_SHOW_HEIGHT);

        emoji->move(QPoint(0,445)); //最大化时的显示位置
        emoji->setMax();
    } else {
        this->setMaximumSize(UICHAT_MIN_SIZE);
        this->setMinimumSize(UICHAT_MIN_SIZE);
        ui->contentShow->setMinimumHeight(MIN_CONTENT_SHOW_HEIGHT);

        emoji->move(QPoint(0,125)); //最小化时的显示位置
        emoji->setMin();

    }
}

void UiChat::on_btn_send_clicked()
{
    closeEmojiShow(); // 关闭表情选择框

    // 正式发送的消息 = 图片 + 纯文本消息
    auto imageList = getImageFromHtml( ui->contentEdit->toHtml());
    QString text = ui->contentEdit->toPlainText();
    if (imageList.isEmpty() && text.isEmpty())
        return;

    text.replace("\n","<br>"); //替换换行符

    QString msg;
    // 多图片情况
    int size = imageList.size();
    for (int i = 0; i < size; ++i){
        msg.append(imageList.at(i));
        if(i != size-1)
            msg.append("<br>");
    }
    // 补上文本消息
    msg += text;

    sendTimeTip(); //时间提示
    showSelfMsg(sideType,selfNickname,selfPicture,msg);

    // 发送显消
    emit sendChatMsg(msg);// 文本消息

    ui->contentEdit->clear(); //清空数据
}

void UiChat::on_btn_emotion_clicked()
{
    emojiShow = !emojiShow;
    if(emojiShow)
        emoji->show();
    else
        emoji->hide();
}

// 发送图片
void UiChat::on_btn_image_clicked()
{
    // *.gif *.jpeg
    QStringList filePathList = QFileDialog::getOpenFileNames(this, "打开", QDir::currentPath(),  "图片(*.jpg *.png)");//单图片
    //没网络情况
    if(!networkNormal)
        return;

    for (int i = 0; i < filePathList.size(); ++i) {
        //设置textBrowser光标到最后
        QTextCursor cursor = ui->contentEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->contentEdit->setTextCursor(cursor);

        //读取图片,计算适当的宽度和高度，并且缩放到合适比例
        QImage image;
        image.load(filePathList.at(i));
        int w = image.width();
        int h = image.height();
        if(w > IMAGE_WIDTH){
            qreal ratio = w/IMAGE_WIDTH;
            w /= ratio;
            h /= ratio;
        }
        QImage scaledImage = image.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation); // 缩放图像
        QPixmap pixmap = QPixmap::fromImage(scaledImage); // 将 QImage转换为 QPixmap

        // 将 QPixmap 转换为 Base64 编码的二进制数据
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        buffer.close();

        // 将 Base64 编码的二进制数据转换为 QString
        QString base64 = QString::fromLatin1(byteArray.toBase64().data());

        // 设置 QTextImageFormat 属性
        QTextImageFormat imageFormat;
        imageFormat.setName("data:image/png;base64," + base64);
        ui->contentEdit->insertHtml(QString("<img  width=\"%1\" height=\"%2\"  "
                                            "src=\"data:image/png;base64, %3 \"/>").arg(QString::number(w), QString::number(h), base64));
//        QString msg = "<img src='data:image/png;base64," + base64Image + "' />";
//        emit sendChatMsg(msg);
    }
    //设置textBrowser光标到最后
    QTextCursor cursor = ui->contentEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->contentEdit->setTextCursor(cursor);

//    ui->contentEdit->insertHtml("<br>"); // 添加一个换行符
}

// 发送文件
void UiChat::on_btn_file_clicked()
{
    QString     filter = "所有文件(*.*);;图片(*.jpg *.png)"; // *.gif *.jpeg
    QStringList filePathList = QFileDialog::getOpenFileNames(this, "打开", QDir::currentPath(), filter);
    //没网络情况
    if(!networkNormal)
        return;
    for (int i = 0; i < filePathList.count(); i++) {
        QFile file(filePathList.at(i));
        QFileInfo info(file);
        QString fileName = info.fileName();
//        QString suffix = info.suffix();
        qint64  fileSize = info.size();
        if(fileSize > 209715200 ) { //209715200bytes = 200MB
            QString tip = QString("[%1]文件过大！无法发送").arg(fileName);
            QMessageBox::information(this,"文件发送",tip,QMessageBox::Ok);
            filePathList.removeAt(i);
        }
    }
    if(filePathList.size() > 0){
        setFileNumTip(filePathList.size());
        sendFileLoading(true);
        emit sendChatFile(filePathList);
    }
}

void UiChat::on_btn_screenshot_clicked()
{
    screenshot->show();
}

void UiChat::on_btn_more_clicked()
{
//    sendFileLoading(true);
}

//自己的消息展示
void UiChat:: showSelfMsg(const int &type,const QString&nickname,const int &index,const QString &msg)
{
    QString picture = getHeadPicture(type,index);

    QString html;
    QString netStyle = "";//网络情况的不同显示
    if(!networkNormal)
        netStyle = "border-bottom: 1px solid red";
    if(type){ //群
        html = QString("<table border=\"0\" align=\"right\" style=\"margin-left:130px\">"
                           " <tr>"
                                "<td align=\"right\" style=\"font-size:12px\"> %1 </td>"
                                "<td rowspan=\"2\"> <img width=\"36\" height=\"36\" src=\"%2\"></td>"
                            "</tr>"
                            "<tr>"
                                "<td style=\"background:rgb(220, 245, 242);%3;padding:6px;\"> %4 </td>"
                            "</tr>"
                           " </table><br>").arg(nickname,picture,netStyle,msg);
    }else{ //好友之间不显示昵称
        html = QString("<table border=\"0\" align=\"right\" style=\"margin-left:130px\">"
                           " <tr>"
                                "<td style=\"background:rgb(220, 245, 242);%2;padding:6px;\"> %3 </td>"
                                "<td > <img width=\"36\" height=\"36\" src=\"%1\"></td>"
                            "</tr>"
                           " </table><br>").arg(picture,netStyle,msg);
    }
    this->insertHtml(html);
}

//对方的消息展示
void UiChat::showSideMsg(const int &type,const QString&nickname,const int &index,const QString &msg)
{
    QString picture = getHeadPicture(type,index);

    QString html;
    QString netStyle = "";//网络情况的不同显示
    if(!networkNormal)
        netStyle = "border-bottom: 1px solid red";

    if(type){  //群
        html = QString("<table border=\"0\" align=\"left\" style=\"margin-right:130px\">"
                           " <tr>"
                                "<td rowspan=\"2\"> <img width=\"36\" height=\"36\" src=\"%1\"></td>"
                                "<td align=\"left\" style=\"font-size:12px\"> %2 </td>"
                            "</tr>"
                            "<tr>"
                                "<td style=\"background:rgb(245, 246, 247);%3;padding:6px;\"> %4 </td>"
                            "</tr>"
                            "</table><br>").arg(picture,nickname,netStyle,msg);
    }else{  //好友之间不显示昵称
        html = QString("<table border=\"0\" align=\"left\" style=\"margin-right:130px\">"
                           " <tr>"
                                "<td > <img width=\"36\" height=\"36\" src=\"%1\"></td>"
                                "<td style=\"background:rgb(245, 246, 247);%2;padding:6px;\"> %3 </td>"
                            "</tr>"
                            "</table><br>").arg(picture,netStyle,msg);
    }
    this->insertHtml(html);
}

QList<QString> UiChat::getImageFromHtml(const QString &html)
{
//    qDebug() << html;
    QList<QString> imageList;
    QRegularExpression re(R"(<img[^>]*src=[\"']data:image/(png|jpg|jpeg);base64,([^"]*)[\"'][^>]*>)",
                          QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = re.globalMatch(html); //获取规则表达式迭代器
    while(it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString base64 = match.captured(2);

        // 图片缩放合适比例
        QByteArray byte = QByteArray::fromBase64(base64.toLatin1());
        QImage image;
        image.loadFromData(byte);
        int w = image.width();
        int h = image.height();
        if(w > IMAGE_WIDTH){
            qreal ratio = w/IMAGE_WIDTH;
            w /= ratio;
            h /= ratio;
        }
        imageList.append(QString("<img  width=\"%1\" height=\"%2\"  "
                                 "src=\"data:image/png;base64, %3 \"/>").arg(QString::number(w), QString::number(h), base64));
    }
    return imageList;
}

// 保存消息里面的图片
void UiChat::saveImageFromMsg(const QString &msg)
{
    Q_UNUSED(msg);
    return;
//    QRegularExpression re(R"(<img[^>]*src=[\"']data:image/(png|jpg|jpeg);base64,([^"]*)[\"'][^>]*>)",
//                          QRegularExpression::CaseInsensitiveOption);
//    QRegularExpressionMatchIterator it = re.globalMatch(msg); //获取规则表达式迭代器
//    while(it.hasNext()) {
//        QRegularExpressionMatch match = it.next();
//        QString base64 = match.captured(2);

//        // 图片缩放合适比例
//        QByteArray byte = QByteArray::fromBase64(base64.toLatin1());
//        QString currDateTime = QDate::currentDate().toString("yyyy-MM-dd.mm:hh:ss");

//        QString fileName = QString("%1/%2.png").arg(IMAGE_PATH(selfId),currDateTime);
//        QFile file(fileName);
//        if(file.open(QFile::WriteOnly)){
//            file.write(byte);
//        }
    //    }
}

//检测消失是否还有图片
bool UiChat::messageHasImage(const QString &msg)
{
   QRegularExpression re(R"(<img[^>]*src=[\"']data:image/(png|jpg|jpeg);base64,([^"]*)[\"'][^>]*>)",
                         QRegularExpression::CaseInsensitiveOption);
   QRegularExpressionMatch match = re.match(msg);
   if (match.hasMatch()) {
       return true;
   }
   return false;
}

void UiChat::recvTimeTip(const QString &sendTime)
{
    // 第一次为当前时间
    if(lastSendDateTime.isNull())
        lastSendDateTime = QDateTime::currentDateTime();
    if(lastRecvDateTime.isNull())
        lastRecvDateTime =  QDateTime::currentDateTime();

    QDateTime currRecvDateTime = QDateTime::fromString(sendTime,"yyyy-MM-dd hh:mm:ss"); //当前消息时间

    //上次接收时间
    int recvMin = lastRecvDateTime.secsTo(currRecvDateTime)/60; //计算分钟数
    if(MSG_DURATION_TIP > recvMin){
        lastRecvDateTime = currRecvDateTime; //更新上次时间
        return;
    }
    //上次发送时间
    int sendMin = lastSendDateTime.secsTo(currRecvDateTime)/60; //计算分钟数
    if(MSG_DURATION_TIP > sendMin)
        return;

    lastRecvDateTime = currRecvDateTime; //更新接收时间

    QString dateTime = currRecvDateTime.toString("yyyy年MM月dd日 hh:mm:ss");
    QString html = QString("<table border=\"0\" align=\"center\">"
                        "<tr>"
                            "<td style=\"background:rgb(245, 246, 247);font-size:12px;padding:4px;\"> %1 </td>"
                        "</tr>"
                        "</table><br>").arg(dateTime);
    insertHtml(html);
}

void UiChat::sendTimeTip()
{
    // 第一次为当前时间
    if(lastSendDateTime.isNull())
        lastSendDateTime = QDateTime::currentDateTime();
    if(lastRecvDateTime.isNull())
        lastRecvDateTime =  QDateTime::currentDateTime();

    QDateTime currSendDateTime = QDateTime::currentDateTime(); //当前消息时间

    //上次发送时间
    int sendMin = lastSendDateTime.secsTo(currSendDateTime)/60; //计算分钟数
    if(MSG_DURATION_TIP > sendMin){
        lastSendDateTime = currSendDateTime; //更新发送时间
        return;
    }
    //上次接收时间
    int recvMin = lastRecvDateTime.secsTo(currSendDateTime)/60; //计算分钟数
    if(MSG_DURATION_TIP > recvMin)
        return;

    lastSendDateTime = currSendDateTime; //更新发送时间

    QString dateTime = currSendDateTime.toString("yyyy年MM月dd日 hh:mm:ss");
    QString html = QString("<table border=\"0\" align=\"center\">"
                        "<tr>"
                            "<td style=\"background:rgb(245, 246, 247);font-size:12px;padding:4px;\"> %1 </td>"
                        "</tr>"
                        "</table><br>").arg(dateTime);
    insertHtml(html);
}

void UiChat::insertHtml(const QString &html)
{
    ui->contentShow->moveCursor(QTextCursor::End);//插入数据前记得移动光标
    ui->contentShow->insertHtml(html);
    ui->contentShow->moveCursor(QTextCursor::End);//第二次是为了消息与滚动条同步

    //如果处于表情选择中，不应该关闭
    if(emojiShow==true)
        emoji->show();
    else
        ui->contentEdit->setFocus(); //继续输入
}

void UiChat::sendFileLoading(const bool &state)
{
    if(state){
        movie->start();
        ui->widget_loading->setVisible(state);
    }else{
        ui->widget_loading->setVisible(state);
        movie->stop();
        ui->lab_loadingTip->clear();
    }
}

void UiChat::on_btn_hideSlide_clicked()
{
    closeEmojiShow();
    hideState = !hideState;
    if(hideState){
        ui->widget_info->setVisible(hideState);
        ui->btn_hideSlide->setToolTip("折叠");
        ui->btn_hideSlide->setIcon(QIcon(":/img/arrow-right.png"));
    } else{
        ui->widget_info->setVisible(hideState);
        ui->btn_hideSlide->setToolTip("展开");
        ui->btn_hideSlide->setIcon(QIcon(":/img/arrow-left.png"));
    }
}

QString UiChat::getHeadPicture(const int type,int picture)
{
    QString _picture;
    // 群聊
    if(picture <= 0){
        if(type){
            _picture = ":/img/head/group.png"; //默认头像
        }else{
            _picture = ":/img/head/person.png"; //默认头像
        }
    }else{
        _picture = QString(":/img/head/%1.png").arg(QString::number(picture));

    }
    return _picture;
}
