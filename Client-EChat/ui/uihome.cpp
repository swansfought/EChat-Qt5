#include "ui/uihome.h"

#include "ui_uihome.h"

UiHome::UiHome(QWidget *parent) :
    QWidget(parent),
    isDrag(false),
    movie(nullptr),
    createGroupPicture(0),
    editingSelfInfo(false),
    editingGrpInfo(false),
    editingMyGrpName(false),
    db(nullptr),
    file(nullptr),
    headPicture(nullptr),
    selectingPicture(false),
    ui(new Ui::UiHome)
{
    ui->setupUi(this);

    // 设置窗口属性
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowTitle("EChat");
//    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->widget_top->installEventFilter(this);
    ui->widget_content->installEventFilter(this);//用于头像选择框关闭
    ui->widget_dock->installEventFilter(this);
    ui->line_horizontal->installEventFilter(this);
    ui->btn_plus->installEventFilter(this);
    ui->btn_message->installEventFilter(this);
    ui->btn_contact->installEventFilter(this);
    ui->btn_file->installEventFilter(this);
    ui->btn_config->installEventFilter(this);
    ui->btn_more->installEventFilter(this);
    ui->btn_newPassword->installEventFilter(this);
    ui->btn_sureNewPassword->installEventFilter(this);

    // 设置窗口阴影
    QGraphicsDropShadowEffect *shadow;
    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);               // 设置阴影距离
    shadow->setColor(QColor(55, 55, 55));  // 设置阴影颜色
    shadow->setBlurRadius(5);              // 设置阴影圆角
    ui->widget_homeBackground->setGraphicsEffect(shadow);

    // 加载样式表
    QFile styleFile(":/qss/home.qss");
    if(styleFile.open(QFile::ReadOnly)){
        QTextStream stream(&styleFile);
        QString     styleSheet = styleFile.readAll();
        this->setStyleSheet(styleSheet);
        styleFile.close();
    }

    db = DataBase::getInstance();
    file = File::getInstance();
    //头像选择框
    headPicture = new HeadPicture(this);
    headPicture->hide();
    headPicture->move(QPoint((this->width()-335)/2, 200)); //移动至指定位置
    connect(headPicture,&HeadPicture::pictureIndex,this,&UiHome::on_setHeadPicture);

    //重连接加载图
    movie = new QMovie(":/img/ghost.gif",QByteArray(),this);
    ui->lab_connecting->setMovie(movie);
    ui->widget_connecting->setVisible(false);

    //dock
    dock.insert(0,ui->btn_message);
    dock.insert(1,ui->btn_contact);
    dock.insert(2,ui->btn_file);
    dock.insert(3,ui->btn_config);
    dock.insert(4,ui->btn_more);

    connect(ui->line_seekInput,&QLineEdit::returnPressed,this,[=](){ on_btn_seek_clicked(); });
    connect(ui->btn_hclose, &QPushButton::clicked, this, [=]() {
        closeHeadPicture(); //清除头像选择框
        this->close();
    });
    connect(ui->btn_hmin, &QPushButton::clicked, this, [=]() {
        this->showMinimized(); //清除头像选择框
        closeHeadPicture();
    });
    connect(ui->btn_hmax, &QPushButton::clicked, this, [=]() {
        if (this->windowState() == Qt::WindowMaximized) {
            this->showNormal();
            emit uiChatMax(false);
        } else {
            this->showMaximized();
            emit uiChatMax(true);
        }
        headPicture->move(QPoint((this->width()-335)/2, 200)); //重新计算指定位置
    });
    // 密码管理左侧dock栏按钮响应操作
    connect(ui->btn_account,&QPushButton::clicked,this,[=]{ ui->scroll_config->verticalScrollBar()->setSliderPosition(0); });
    connect(ui->btn_password,&QPushButton::clicked,this,[=]{ui->scroll_config->verticalScrollBar()->setSliderPosition(220); });
    connect(ui->btn_notify,&QPushButton::clicked,this,[=]{ ui->scroll_config->verticalScrollBar()->setSliderPosition(385); });
    connect(ui->btn_fileManage,&QPushButton::clicked,this,[=]{ ui->scroll_config->verticalScrollBar()->setSliderPosition(516); });
    connect(ui->btn_update,&QPushButton::clicked,this,[=]{ ui->scroll_config->verticalScrollBar()->setSliderPosition(516); });
    connect(ui->btn_about,&QPushButton::clicked,this,[=]{  ui->scroll_config->verticalScrollBar()->setSliderPosition(516); });
    connect(ui->line_newPassword,&QLineEdit::textChanged,this,[=] { ui->line_sureNewPassword->clear(); });

    //初始化基础控件
    initBaseContact();

    QMenu *menu = new QMenu(this);
    menu->addAction("加好友/群聊",[=]{ //跳转到加好友/群界面
        on_btn_contact_clicked();
        on_btn_friend_clicked();
        ui->btn_friend->setChecked(true);
        ui->btn_seekFriend->setChecked(true);
        contactCliked(ClikedType::AddContact,friends.find(1).value()->item);
    },0);
    menu->addAction("创建群聊",[=]{ //跳转到加好友/群界面
        on_btn_contact_clicked();
        on_btn_group_clicked();
        ui->btn_group->setChecked(true);
        ui->btn_seekGroup->setChecked(true);
        contactCliked(ClikedType::AddGroup,groups.find(0).value()->item);
    },0);
    menu->setWindowFlags(menu->windowFlags()| Qt::FramelessWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);
    ui->btn_plus->setMenu(menu);
    ui->btn_plus->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->line_realTimeSearch,&QLineEdit::textChanged,this,[=]{ closeHeadPicture(); }); //用于头像选择框关闭
    connect(ui->line_groupName,&QLineEdit::textChanged,this,[=]{ closeHeadPicture(); }); //用于头像选择框关闭

    // 初始默认页
    ui->stacked_home->setCurrentIndex(PAGE_HOME_MSG);
    lastDockIndex = PAGE_HOME_MSG;
    ui->stacked_message->setCurrentIndex(PAGE_HOME_MSG_DEFAULT);
    ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_DEFAULT);
    ui->stacked_contactShow->setCurrentIndex(PAGE_CONTACT_TYPE_FRIEND);
    ui->stackedWidget_file->setCurrentIndex(0);
    ui->btn_friend->setChecked(true);
    contactType = PAGE_CONTACT_TYPE_FRIEND;

    // -----------------------
    ui->btn_friendFile->setVisible(false);
    ui->btn_groupFile->setVisible(false);
    setInputRegExp(); //设置正则表达式

    // 获取聊天对象在线状态 & 好友/群聊昵称和头像(更新本地数据)
    // 并且提供消息闪烁提示
    updateTimer = new QTimer(this);
    updateTimer->setInterval(10*1000); //
    connect(updateTimer,&QTimer::timeout,this,[=] {
        emit resourceRequest(815,0,0,"");
//        emit resourceRequest(816,0,0,"");

        // 好友/群里消息3min更新一次
        static int updateFlag = 0;
        if(18 == updateFlag){
            emit resourceRequest(816,0,0,"");
            updateFlag = 0;
        }
        ++updateFlag;
    });
}

UiHome::~UiHome()
{
    qDebug() << "UiHome资源释放完成！";
    delete ui;
}

void UiHome::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(245, 246, 247));  // 245, 246, 247
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(QRect(1, 1, this->width() - 2, this->height() - 2), 6, 6);
    QWidget::paintEvent(event);
}

void UiHome::mousePressEvent(QMouseEvent *event)
{
    offset = event->globalPos() - this->pos();
    QWidget::mousePressEvent(event);
}

void UiHome::mouseReleaseEvent(QMouseEvent *event)
{
    offset = QPoint();
    QWidget::mouseReleaseEvent(event);
}

void UiHome::mouseMoveEvent(QMouseEvent *event)
{
    if (!isDrag || QPoint() == offset)
        return;
    if (this->windowState() == Qt::WindowMaximized) {
        emit uiChatMax(false);
        this->showNormal();
        offset = QPoint();
        headPicture->move(QPoint((this->width()-335)/2, 200)); //移动至指定位置
    } else
        this->move(event->globalPos() - offset);
    QWidget::mouseMoveEvent(event);
}

bool UiHome::eventFilter(QObject *watched, QEvent *event)
{
    // 鼠标点击事件，关闭头像选择框
    // 内容部分点击就关闭
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); //事件转换
        if( Qt::LeftButton == mouseEvent->button()){
            if(selectingPicture && dynamic_cast<QWidget*>(watched) == ui->widget_content){
                closeHeadPicture();
            }
            //关闭表情选择框
            if(dynamic_cast<QWidget*>(watched) == ui->widget_content || dynamic_cast<QWidget*>(watched) == ui->widget_top){
                if(currChatObjInfo != nullptr)
                    currChatObjInfo->uiChat->closeEmojiShow();
            }
        }
    }
    else if(event->type() == QEvent::Enter){
        // 拖动设置
        if(dynamic_cast<QWidget*>(watched) == ui->widget_dock || dynamic_cast<QWidget*>(watched) == ui->line_horizontal)
            this->isDrag = false;
        else if(dynamic_cast<QWidget*>(watched) == ui->widget_top)
            this->isDrag = true;

        //按钮浮动变亮提示
        if(dynamic_cast<QPushButton*>(watched) == ui->btn_plus){
            ui->btn_plus->setIcon(QIcon(":/img/addContanct-hover.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_message){
            if(dock.find(lastDockIndex).value() != ui->btn_message)
                ui->btn_message->setIcon(QIcon(":/img/msg-hover.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_contact){
            if(dock.find(lastDockIndex).value() != ui->btn_contact)
                ui->btn_contact->setIcon(QIcon(":/img/contact-hover.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_file){
            if(dock.find(lastDockIndex).value() != ui->btn_file)
                ui->btn_file->setIcon(QIcon(":/img/folder-hover.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_config){
            if(dock.find(lastDockIndex).value() != ui->btn_config)
                ui->btn_config->setIcon(QIcon(":/img/setting-hover.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_more){
            if(dock.find(lastDockIndex).value() != ui->btn_more)
                ui->btn_more->setIcon(QIcon(":/img/more-hover.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_newPassword){
            ui->btn_newPassword->setIcon(QIcon(":/img/pwdOpen.png"));
            ui->line_newPassword->setEchoMode(QLineEdit::Normal);
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_sureNewPassword){
            ui->btn_sureNewPassword->setIcon(QIcon(":/img/pwdOpen.png"));
            ui->line_sureNewPassword->setEchoMode(QLineEdit::Normal);
        }
    }
    else if(event->type() == QEvent::Leave){
        // 拖动设置
        if(dynamic_cast<QWidget*>(watched) == ui->widget_dock || dynamic_cast<QWidget*>(watched) == ui->line_horizontal)
            this->isDrag = true;
        else if(dynamic_cast<QWidget*>(watched) == ui->widget_top)
            this->isDrag = false;

        //按钮浮动变亮恢复
        if(dynamic_cast<QPushButton*>(watched) == ui->btn_plus){
             ui->btn_plus->setIcon(QIcon(":/img/addContanct-normal.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_message){
            if(dock.find(lastDockIndex).value() != ui->btn_message)
                ui->btn_message->setIcon(QIcon(":/img/msg-normal.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_contact){
            if(dock.find(lastDockIndex).value() != ui->btn_contact)
                ui->btn_contact->setIcon(QIcon(":/img/contact-normal.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_file){
            if(dock.find(lastDockIndex).value() != ui->btn_file)
                ui->btn_file->setIcon(QIcon(":/img/folder-normal.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_config){
            if(dock.find(lastDockIndex).value() != ui->btn_config)
                ui->btn_config->setIcon(QIcon(":/img/setting-normal.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_more){
            if(dock.find(lastDockIndex).value() != ui->btn_more)
                ui->btn_more->setIcon(QIcon(":/img/more-normal.png"));
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_newPassword){
            ui->btn_newPassword->setIcon(QIcon(":/img/pwdClose.png"));
            ui->line_newPassword->setEchoMode(QLineEdit::Password);
        }else if(dynamic_cast<QPushButton*>(watched) == ui->btn_sureNewPassword){
            ui->btn_sureNewPassword->setIcon(QIcon(":/img/pwdClose.png"));
            ui->line_sureNewPassword->setEchoMode(QLineEdit::Password);
        }
    }
    return QWidget::eventFilter(watched,event);
}

void UiHome::on_setHeadPicture(const int index)
{
    QString picture = QString(":/img/head/%1.png").arg(QString::number(index));
    QPixmap pixmap;
    pixmap.load(picture);

    //先判断是否为创建群聊选择图片
    auto info = groups.find(0).value();
    if(info->isSelect){
        createGroupPicture = index;
        ui->lab_groupPicture->setPixmap(pixmap); //创建群聊设置的头像
        return;
    }

    QJsonObject obj;
    obj.insert("picture",index);

    // 好友界面
    if(0 == contactType){
        ui->lab_headpicture->setPixmap(pixmap); //自己的头像
        ui->lab_selfPicture->setPixmap(pixmap);
        selfPicture = index;
        obj.insert("type",0);
    } else{
        ui->lab_grpPicture->setPixmap(pixmap); //群聊头像

        // 设置群聊列表中的头像
        auto info = groups.find(currContactGrpID).value();
        info->item->setPicture(1,index);

        obj.insert("type",1);
        obj.insert("groupId",currContactGrpID);
    }
    emit resourceRequest(706,0,0,obj); //同步到服务器中
}

void UiHome::on_loginSuccess()
{
    // 拿到聊天对象列表、好友列表，群聊列表去初始化控件
    // 先初始化控件，其他的初始化数据在其之后
    emit resourceRequest(810, 0, 0,"");

    //加载配置界面数据
    ui->btn_autoLogin->setChecked( file->getAutoLogin() );
    ui->btn_remainPwd->setChecked( file->getRemainPwd() );

    ui->btn_newNotifyVoice->setChecked( file->getNewNotifyVoice() );
    ui->btn_groupNotifyVoice->setChecked( file->getGroupNotifyVoice() );
    ui->btn_autoUpdate->setChecked( file->getAutoUpdate() );
    ui->plain_filePath->setPlainText( FILE_PATH(selfId) );
    file->updateFilePath(FILE_PATH(selfId));

    initFileList();//初始化文件列表

    db->addLocalUser(0,selfId,selfNickname,selfPicture);
    updateTimer->start();
}

void UiHome::on_fileSendFinish(const ChatObjInfo* chatObjInfo,const QString &fileName,const QString sql)
{
    chatObjInfo->uiChat->removeFileNumTip();
    chatObjInfo->uiChat->addFileMsg(fileName,selfId);
    chatObjInfo->item->setRecentChatMsg( "文件消息");
    if(!sql.isEmpty()){
        QSqlQuery query;
        if(!query.exec(sql))
            qDebug()<<"本地文件消息存储失败...";
    }
}

void UiHome::on_connectState(bool state)
{
    if (state) {
        ui->widget_connecting->setVisible(false);
        movie->stop();
        ui->lab_connectingTip->clear();
        ui->lab_connecting->setToolTip("");
    } else {
        movie->start();
        ui->widget_connecting->setVisible(true);
        ui->lab_connectingTip->setText("网络已断开...");
        ui->lab_connecting->setToolTip("没网了~");
    }
}

// 发送消息
void UiHome::on_sendChatMsg(const QString &msg)
{
    auto it = chatObjects.find(currChatObjectID);
    if(it == chatObjects.end()){
        QMessageBox::warning(this,"发送消息","无法获取当前好友ID！",QMessageBox::Ok);
        return;
    }
    int frdId = 0;
    int grpId = 0;
    int type = 0;
    QString sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if (it.value()->isFrdId){
        emit sendChatMsg(msg, currChatObjectID, 0, sendTime);
        frdId = currChatObjectID;
        it.value()->item->setRecentChatTime(sendTime);
        if(msg.length() > 12)
            it.value()->item->setRecentChatMsg(msg.left(12));
        else
            it.value()->item->setRecentChatMsg(msg);
    }
    else if (it.value()->isGrpId){
        emit sendChatMsg(msg, 0, currChatObjectID, sendTime);
        grpId = currChatObjectID;
        type = 1;
        it.value()->item->setRecentChatTime(sendTime);
        if(msg.length() > 12)
            it.value()->item->setRecentChatMsg(msg.left(12));
        else
            it.value()->item->setRecentChatMsg(msg);
    }
    //最近聊天的置到最前面
    ui->vLayout_message->removeWidget(it.value()->item);
    ui->vLayout_message->insertWidget(0,it.value()->item);

    //写入本地数据库中，不断网才去，因为断网发的消息是假的
    if(!networkNormal)
        return;
    auto db = DataBase::getInstance();
    db->addLocalMsg(msg, selfId, selfPicture,frdId, grpId, type, sendTime);
}

// 发送文件
void UiHome::on_sendChatFile(const QStringList &filePathList)
{
    for (auto it = chatObjects.begin(); it != chatObjects.end(); ++it) {
        if (!it.value()->isSelect)
            continue;
        int type = 0;
        if (it.value()->isGrpId)
            type = 1;
        if(it.value()->item->getOnlineState()){
            if(type)
                emit sendChatFile(it.value(),filePathList, 0, it.key());
            else
                emit sendChatFile(it.value(),filePathList, it.key(), 0);
        }else{
            it.value()->uiChat->setFileNumTip(0); //去掉提示
//            // 对方不在线不发送文件
//            for (int j = 0; j < filePathList.size(); ++j)
//                it.value()->uiChat->removeFileNumTip();
        }
        break;
    }
}

void UiHome::dockIconChanged(const int &index)
{
    if(currChatObjInfo != nullptr)
        currChatObjInfo->uiChat->closeEmojiShow();//关闭表情选择框

    if(index == lastDockIndex)
        return;
    closeHeadPicture();// 用于头像选择框关闭

    //选中当前项
    auto it = dock.find(index).value();
    if(0 == index)
        it->setIcon(QIcon(":/img/msg-clicked.png"));
    else if(1 == index)
        it->setIcon(QIcon(":/img/contact-clicked.png"));
    else if(2 == index)
        it->setIcon(QIcon(":/img/folder-clicked.png"));
    else if(3 == index)
        it->setIcon(QIcon(":/img/setting-clicked.png"));
    else if(4 == index)
        it->setIcon(QIcon(":/img/more-clicked.png"));

    //恢复上次选中的
    auto _it = dock.find(lastDockIndex).value();
    if(0 == lastDockIndex)
        _it->setIcon(QIcon(":/img/msg-normal.png"));
    else if(1 == lastDockIndex)
        _it->setIcon(QIcon(":/img/contact-normal.png"));
    else if(2 == lastDockIndex)
        _it->setIcon(QIcon(":/img/folder-normal.png"));
    else if(3 == lastDockIndex)
        _it->setIcon(QIcon(":/img/setting-normal.png"));
    else if(4 == lastDockIndex)
        _it->setIcon(QIcon(":/img/more-normal.png"));

    lastDockIndex = index;
}

// 头像
void UiHome::on_btn_headPic_clicked() {}

// 信息
void UiHome::on_btn_message_clicked()
{
    dockIconChanged(PAGE_HOME_MSG);
    ui->stacked_home->setCurrentIndex(PAGE_HOME_MSG);
}

// 联系人
void UiHome::on_btn_contact_clicked()
{
    dockIconChanged(PAGE_HOME_CONTACT);
    ui->stacked_home->setCurrentIndex(PAGE_HOME_CONTACT);
    ui->stacked_contactShow->setCurrentIndex(contactType);
}

// 文件
void UiHome::on_btn_file_clicked()
{
    clearFiles(); //清除文件消息
    initFileList();//初始化文件列表

    dockIconChanged(PAGE_HOME_FILE);
    ui->stacked_home->setCurrentIndex(PAGE_HOME_FILE);
}

// 配置
void UiHome::on_btn_config_clicked()
{
    // 清除上次遗留的密码
    ui->line_newPassword->clear();
    ui->line_sureNewPassword->clear();

    dockIconChanged(PAGE_HOME_CONFIG);
    ui->stacked_home->setCurrentIndex(PAGE_HOME_CONFIG);
}

// 更多
void UiHome::on_btn_more_clicked()
{
    ui->textEdit_suggest->clear();
    dockIconChanged(PAGE_HOME_MORE);
    ui->stacked_home->setCurrentIndex(PAGE_HOME_MORE);
    ui->textEdit_suggest->setFocus();
}

void UiHome::on_btn_friend_clicked()
{
    closeHeadPicture();
    ui->stacked_contact->setCurrentIndex(friendOpPage);
    ui->stacked_contactShow->setCurrentIndex(PAGE_CONTACT_TYPE_FRIEND);
    contactType = PAGE_CONTACT_TYPE_FRIEND;
}

void UiHome::on_btn_group_clicked()
{
    closeHeadPicture();
    ui->stacked_contact->setCurrentIndex(groupOpPage);
    ui->stacked_contactShow->setCurrentIndex(PAGE_CONTACT_TYPE_GROUP);
    contactType = PAGE_CONTACT_TYPE_GROUP;
}

void UiHome::on_btn_seek_clicked()
{
    QString input = ui->line_seekInput->text();
    if(input.isEmpty())
        return;

//    clearSeekInfo(); //清除上次信息

    if (input.size() != 7) {
        ui->lab_seekTip->setText("EC号输入格式有误！");
        return;
    }
    if (ui->btn_seekFriend->isChecked())
        emit resourceRequest(808, input.toInt(), 0,"");
    else
        emit resourceRequest(804, 0, input.toInt(),"");
}

void UiHome::on_btn_add_clicked()
{
    int id = ui->lab_seekId->text().toInt();
    QJsonObject obj;
    obj.insert("ps",ui->line_addPs->text());
    if (ui->btn_seekFriend->isChecked())
        emit resourceRequest(901, id, 0,obj);
    else
        emit resourceRequest(901, 0, id,obj);
}

void UiHome::on_btn_addGroupPicture_clicked()
{
    headPicture->setPictureType(1);
    headPicture->show();
    selectingPicture = true;
}

void UiHome::on_btn_edit_clicked()
{
    closeHeadPicture();
    ui->btn_edit->setText("完成编辑");
    ui->cBox_sex->setEnabled(!editingSelfInfo);
    ui->line_nickname->setReadOnly(editingSelfInfo);
    ui->text_selfSay->setReadOnly(editingSelfInfo);
    ui->line_phone->setReadOnly(editingSelfInfo);
    ui->line_region->setReadOnly(editingSelfInfo);
    ui->line_email->setReadOnly(editingSelfInfo);
    ui->line_nickname->setFocus(); //设置焦点
    //设置编辑样式
    ui->line_nickname->setStyleSheet(LINE_EDIT_START);
    ui->text_selfSay->setStyleSheet(LINE_EDIT_START);
    ui->line_phone->setStyleSheet(LINE_EDIT_START);
    ui->line_region->setStyleSheet(LINE_EDIT_START);
    ui->line_email->setStyleSheet(LINE_EDIT_START);

    editingSelfInfo = !editingSelfInfo;
    if(editingSelfInfo)
        return;

    QString nickname = ui->line_nickname->text();
    if(nickname.isEmpty()){
        QMessageBox::information(this,"信息修改","[个人]昵称填写不完整！",QMessageBox::Ok);
        return;
    }
    QString selfSay = ui->text_selfSay->toPlainText();
    if(selfSay.size() > 250){
        QMessageBox::information(this,"信息修改","[个人]标签字数过多，需<=250个！",QMessageBox::Ok);
        return;
    }
    QString sex = ui->cBox_sex->currentText();
    QString phone = ui->line_phone->text();
    QString region = ui->line_region->text();
    QString email = ui->line_email->text();

    ui->btn_edit->setText("编辑信息"); //恢复提示
    QJsonObject obj;
    obj.insert("nickname",nickname);
    obj.insert("selfSay",selfSay);
    obj.insert("sex",sex);
    obj.insert("phone",phone);
    obj.insert("region",region);
    obj.insert("email",email);
    emit resourceRequest(701,0,selfId,obj);

    //恢复正常样式
    ui->line_nickname->setStyleSheet(LINE_EDIT_FINISH);
    ui->text_selfSay->setStyleSheet(LINE_EDIT_FINISH);
    ui->line_phone->setStyleSheet(LINE_EDIT_FINISH);
    ui->line_region->setStyleSheet(LINE_EDIT_FINISH);
    ui->line_email->setStyleSheet(LINE_EDIT_FINISH);
}

void UiHome::on_btn_editPicture_clicked()
{
    headPicture->setPictureType(0);
    headPicture->show();
    selectingPicture = true;
}

void UiHome::on_btn_sendMsg_clicked()
{
    closeHeadPicture();
    if(editingSelfInfo){
        QMessageBox::information(this,"信息修改","[个人]请先保存编辑信息！",QMessageBox::Ok);
        return;
    }
    auto it = chatObjects.find(currContactFrdID);
    // 已存在改聊天对象，直接跳转过去显示
    if(chatObjects.end() != it){
        ui->stacked_message->setCurrentWidget(it.value()->uiChat);
        ui->stacked_home->setCurrentIndex(PAGE_HOME_MSG);
        ui->btn_message->setChecked(true);
        buttonExclusive(ButtonType::ChatObject,it.value()->item);
    }else{
        int frdId = ui->line_id->text().toInt();

        auto info = createChatObject(frdId,0); //创建新的聊天窗口
        initLocalMsg(info,frdId); //加载本地数据

        //跳转过去显示
        ui->stacked_message->setCurrentWidget(info->uiChat);//聊天界面
        buttonExclusive(ButtonType::ChatObject,info->item);
        ui->stacked_home->setCurrentIndex(PAGE_HOME_MSG);//消息界面
        currContactFrdID = frdId;
    }
    dockIconChanged(0);//dock栏恢复
}

//删除好友
void UiHome::on_btn_deleteContact_clicked()
{
    int frdId = ui->line_id->text().toInt();
    QString nickname = ui->line_nickname->text();
    QString tip = QString("EC号：%1\n昵称：%2\n确定要[删除]好友吗?").arg(QString::number(frdId),nickname);
    QMessageBox::StandardButton ret = QMessageBox::information(this,"删除好友",tip,QMessageBox::Yes | QMessageBox::No);
    if(QMessageBox::No == ret || QMessageBox::Close == ret){
        return;
    }
    emit resourceRequest(601,frdId,0,"");
}

void UiHome::on_btn_sureAddGourp_clicked()
{
    QString groupName = ui->line_groupName->text();
    if(groupName.isEmpty())
        return;

    QJsonObject obj;
    obj.insert("groupName",groupName);
    obj.insert("picture",createGroupPicture);
    obj.insert("leaderId",selfId);
    QJsonArray arr;
    QString id;
    // 获取选中的群成员
    for(auto it=selectGrpMems.begin(); it!=selectGrpMems.end(); it++){
         if(!it.value()->isChecked())
             continue;
         id = it.value()->text().split("[").at(1); //id]
         id = id.mid(0,id.size()-1);//id
         arr.append(id.toInt());
     }
     obj.insert("members",arr);
     obj.insert("memberCount",arr.size());

     emit resourceRequest(506, 0, 0, obj);
}

void UiHome::on_btn_cancelAddGroup_clicked()
{
    ui->lab_groupPicture->setPixmap(QPixmap(":/img/head/group.png"));
    ui->line_groupName->clear();
    createGroupPicture = 0; //恢复默认图标索引

     //取消选中项
    for(auto it = selectGrpMems.begin(); it != selectGrpMems.end(); ++it){
        if(!it.value()->isChecked())
            continue;
        it.value()->setChecked(false);
    }
}

//删 ? 退 群
void UiHome::on_btn_quitGroup_clicked()
{
    closeHeadPicture();
    int grpId = ui->line_grpId->text().toInt();
//    int leaderId = ui->line_grpLeaderId->text().toInt();
    QStringList toolTipList = ui->tool_grpLeaderInfo->toolTip().split(":");
    int leaderId = toolTipList.at(1).toInt();
    QString nickname = ui->line_grpNickname->text();

    int code;
    QString title,tip;
    if(leaderId != selfId){
        code = 602;
        title = "退出群聊";
        tip = QString("群EC号：%1\n群昵称：%2\n确定要[退出]群聊吗?").arg(QString::number(grpId),nickname);
    }else{
        code = 603;
        title = "删除群聊";
        tip = QString("群EC号：%1\n群昵称：%2\n确定要[删除]群聊吗?").arg(QString::number(grpId),nickname);
    }
    QMessageBox::StandardButton ret = QMessageBox::information(this,title,tip,QMessageBox::Yes | QMessageBox::No);
    if(QMessageBox::No == ret || QMessageBox::Close == ret){
        return;
    }
    emit resourceRequest(code,0,grpId,"");//删群
}

void UiHome::on_btn_grpEdit_clicked()
{
    closeHeadPicture();
    ui->btn_grpEdit->setText("完成编辑");
    ui->line_grpNickname->setReadOnly(editingGrpInfo);
    ui->text_grpIntro->setReadOnly(editingGrpInfo);
    ui->line_grpNickname->setFocus(); //设置焦点
    //设置编辑样式
    ui->line_grpNickname->setStyleSheet(LINE_EDIT_START);

    editingGrpInfo = !editingGrpInfo;
    if(editingGrpInfo)
        return;

    QString nickname = ui->line_grpNickname->text();
    if(nickname.isEmpty()){
        QMessageBox::information(this,"信息修改","[群聊]昵称填写不完整！",QMessageBox::Ok);
        return;
    }
    QString intro = ui->text_grpIntro->toPlainText();
    if(intro.size() > 250){
        QMessageBox::information(this,"信息修改","[群聊]简介字数过多，需<=250个！",QMessageBox::Ok);
        return;
    }
    ui->btn_grpEdit->setText("编辑群聊"); //恢复提示
    QJsonObject obj;
    obj.insert("type",1); //1-群聊信息修改
    obj.insert("nickname",nickname);
    obj.insert("intro",intro);
    emit resourceRequest(703,0,ui->line_grpId->text().toInt(),obj);

    //恢复样式
    ui->line_grpNickname->setStyleSheet(LINE_EDIT_FINISH);
}

void UiHome::on_btn_grpEditMyName_clicked()
{
    ui->btn_grpEditMyName->setIconSize(QSize(18,18));
    ui->btn_grpEditMyName->setIcon(QIcon(":/img/sure.png")); //恢复编辑图标
    ui->line_usrGrpNickname->setReadOnly(editingMyGrpName);
    ui->line_usrGrpNickname->setFocus();
    //设置编辑样式
    ui->line_usrGrpNickname->setStyleSheet(LINE_EDIT_START);

    editingMyGrpName = !editingMyGrpName;
    if(editingMyGrpName)
        return;
    QString nickname = ui->line_usrGrpNickname->text();
    if(nickname.isEmpty()){
        QMessageBox::information(this,"信息修改","[群聊]我的群昵称填写不完整！",QMessageBox::Ok);
        return;
    }
    ui->btn_grpEditMyName->setIcon(QIcon(":/img/edit.png"));
    ui->btn_grpEditMyName->setIconSize(QSize(20,20));
    QJsonObject obj;
    obj.insert("type",0);//0-用户信息修改
    obj.insert("userGroupNickname",nickname);
    emit resourceRequest(703,0,ui->line_grpId->text().toInt(),obj);

    //恢复样式
    ui->line_usrGrpNickname->setStyleSheet(LINE_EDIT_FINISH);
}

void UiHome::on_btn_editGrpPicture_clicked()
{
    headPicture->setPictureType(1);
    headPicture->show();
    selectingPicture = true;
}

void UiHome::on_btn_grpSendMsg_clicked()
{
    closeHeadPicture();
    if(editingGrpInfo){
        QMessageBox::information(this,"信息修改","[群聊]请先保存编辑信息！",QMessageBox::Ok);
        return;
    }
    auto it = chatObjects.find(currContactGrpID);
    if(chatObjects.end() != it){
        ui->stacked_message->setCurrentWidget(it.value()->uiChat);
        ui->stacked_home->setCurrentIndex(PAGE_HOME_MSG);
        ui->btn_message->setChecked(true);
        buttonExclusive(ButtonType::ChatObject,it.value()->item);
    }else{
        int grpId = ui->line_grpId->text().toInt();

        auto info = createChatObject(grpId,1); //创建新的聊天窗口
        initLocalMsg(info,grpId);

        //跳转过去显示
        ui->vLayout_message->insertWidget(0,info->item);//新的对象在最前面(置顶先排除)
        ui->stacked_message->setCurrentWidget(info->uiChat);//聊天界面
        buttonExclusive(ButtonType::ChatObject,info->item);

        ui->stacked_home->setCurrentIndex(PAGE_HOME_MSG);//消息界面
        currContactGrpID = grpId;
    }
    dockIconChanged(0);//dock栏恢复
}

/***********************************************************************
 * @brief: code资源请求回应处理
 * @param: json数据
 * @note:
 ***********************************************************************/
void UiHome::on_resourceReply(const QByteArray &json)
{
    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "on_resourceReply json parse error: " << parseError.errorString();
        return;
    }
    QJsonObject rootObj = doc.object();
    int         code = rootObj["code"].toInt();
    bool        state = rootObj["state"].toBool();
//    if (!state)
//        qDebug() << "code：" << code << "  state：" <<state;
    switch (code) {
    case 103:{ //接受文件
        bool sendState = rootObj["sendState"].toBool();
        QString fileName = QString("%1/%2").arg(FILE_PATH(selfId),rootObj["fileName"].toString());
//            qDebug()<<fileName;
        if(!sendState) {
            QFile file(fileName);
            if(file.open(QFile::Append)){
                QByteArray bytes;
                bytes = rootObj["data"].toString().toUtf8();
                file.write(bytes);
            }
        }
//        else {
//            // 接收完成存储至本地数据库中
//            int type = rootObj["type"].toInt();
//            int receiver = rootObj["receiver"].toInt(); //本身

//            // 添加窗口提示消息
//            int sender = rootObj["sender"].toInt(); //对方
//            QString fileName = rootObj["fileName"].toString();
//            auto it = chatObjects.find(sender);
//            if(it != chatObjects.end())
//                it.value()->uiChat->addFileMsg(fileName,sender); //对方发的

//            QString suffix = rootObj["suffix"].toString();
//            QString sendTime =  rootObj["sendTime"].toString();
//            QString filePath = FILE_PATH(selfId);
//            if(type){
//                QSqlQuery query;
//                query.prepare("INSERT INTO ec_messages(sender,senderPicture,frd_id,type,msg_type,send_time,file_name,suffix,file_path)  "
//                              "VALUES(?,?,?,?,?,?,?,?);");
//                query.bindValue(0,sender);
//                query.bindValue(1,receiver);
//                query.bindValue(2,type);
//                query.bindValue(3,"file");
//                query.bindValue(4,sendTime);
//                query.bindValue(5,fileName);
//                query.bindValue(6,suffix);
//                query.bindValue(7,filePath);
//                if(!query.exec())
//                    qDebug()<<"文件数据 本地数据存储失败...";
//            }else{
//                QSqlQuery query;
//                query.prepare("INSERT INTO ec_messages(sender,frd_id,type,msg_type,send_time,file_name,suffix,file_path)  "
//                              "VALUES(?,?,?,?,?,?,?,?);");
//                query.bindValue(0,sender);
//                query.bindValue(1,receiver);
//                query.bindValue(2,type);
//                query.bindValue(3,"file");
//                query.bindValue(4,sendTime);
//                query.bindValue(5,fileName);
//                query.bindValue(6,suffix);
//                query.bindValue(7,filePath);
//                if(!query.exec())
//                    qDebug()<<"文件数据 本地数据存储失败...";
//            }
//        }
        break;
    }
    case 104:{

        break;
    }
    case 501:  // 增好友
    {
        if(!state)
            return;

        int applicant = rootObj["applicant"].toInt();
//        bool agree = rootObj["agree"].toBool();
        for(auto it=applicants.begin(); it!=applicants.end(); ++it){
            if(it.value()->getApplicantId() != applicant)
                continue;
//            it.value()->setAgree(agree);
            friends.find(0).value()->item->removeApplicants(); //处理一个删除减去提示

            ui->vLayout_friendApply->removeWidget(it.value());
            applicants.remove(it.key());
            it.value()->deleteLater();
            it.value() = nullptr;
        }
        break;
    }
    case 502:  // 增群成员(进群)
    {
        if(!state)
            return;

        int groupId = rootObj["groupId"].toInt();
//        bool agree = rootObj["agree"].toBool();
        for(auto it=applicants.begin(); it!=applicants.end(); ++it){
            if(it.value()->getApplicantId() != groupId)
                continue;

            friends.find(0).value()->item->removeApplicants(); //处理一个删除减去提示

            ui->vLayout_friendApply->removeWidget(it.value());
            applicants.remove(it.key());
            it.value()->deleteLater();
            it.value() = nullptr;
        }
        break;
    }
    case 503: // 接收聊天消息
    {
        if(!state)
            return;

        //拿到对方发送的数据信息
        int type = rootObj["type"].toInt();
        int sideId = 0;
        if(type){ //群聊
            sideId = rootObj["receiver"].toInt();
        }else{
            sideId = rootObj["sender"].toInt();
        }

        auto it = chatObjects.find(sideId);
        QString msgType = rootObj["msgType"].toString();
//        qDebug()<<msgType;

         ChatObjInfo *info;
        if(chatObjects.end() != it){
            info = it.value();
        }else {
            info = createChatObject(sideId,type); //创建聊天缩略图并显示
        }
        info->uiChat->recvChatMsg(rootObj); // 接收消息
        info->item->setRecentChatTime(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        QString msg;
        if(MSG_TEXT == msgType){
            msg = rootObj["message"].toString();
        }else{
            msg = "文件消息";
        }
        if(!info->isSelect)
            info->item->addRecentUnread();
        if(msg.length() > 12)
            info->item->setRecentChatMsg(msg.left(12));
        else
            info->item->setRecentChatMsg(msg);
        this->newMessageTip(type); // 新消息提示

        //最近聊天的置到最前面
        ui->vLayout_message->removeWidget(info->item);
        ui->vLayout_message->insertWidget(0,info->item);
        break;
    }
    case 504:  //发送消息回应
    {
        break;
    }
    case 505:  // 增聊天对象
    {
        break;
    }
    case 506:  // 创建群聊
    {
        if(!state){
             QMessageBox::information(this,"创建EC群","群创建失败，请检查网络配置！",QMessageBox::Ok);
             return;
        }
        int newGroupId = rootObj["groupId"].toInt();

         //添加到群展示列表中
        ContactInfo *info = new ContactInfo;
        info->item = new ItemContact(this);
        info->item->setId(newGroupId);
        info->item->setNickname( ui->line_groupName->text() );
        info->item->setPicture(1,createGroupPicture);

        groups.insert(newGroupId,info);
        ui->vLayout_group->insertWidget(ui->vLayout_group->count() - 1, info->item);  //尾部插入，最后一个是弹簧
        connect(info->item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::Group,info->item); }); //连接信号

        // 提示窗口
        QString text = QString("EC群创建成功！\n账号:%1").arg(QString::number(newGroupId));
        QMessageBox::information(this,"创建EC群",text,QMessageBox::Ok);

        on_btn_cancelAddGroup_clicked();//清除信息
        break;
    }
    case 601: // 删好友
    {
        if (!state)
            return;
        deleteFriend(rootObj["friendId"].toInt());
        break;
    }
    case 602:  // 删群成员(退群)
    {
        if (!state)
            return;
        quitGroup(rootObj["groupId"].toInt());
        break;
    }
    case 603:  // 删除群(群主删群)
    {
        if (!state)
            return;
        quitGroup(rootObj["groupId"].toInt());
        break;
    }
    case 604:  // 删聊天文件
    {
        break;
    }
    case 605:  // 删聊天对象
    {
        break;
    }
    case 703:{ //修改群信息回应
        if (!state)
            return;

        // 0是昵称  1是群消息
        int type = rootObj["type"].toInt();
        if(!type)
            ui->tool_grpLeaderInfo->setText(rootObj["userGroupNickname"].toString());
        break;
    }
    case 704:  // 修改密码
    {
        if (!state){
            QMessageBox::information(this,"修改密码","密码修改失败，请重新尝试！",QMessageBox::Yes);
            return;
        }
        QMessageBox::information(this,"修改密码","密码修改成功，将重新登录！",QMessageBox::Yes);
        modifyPassword = true;
        on_btn_exitLogin_clicked(); //退出登录
        break;
    }
    case 705:  // 重置密码
    {
        if (!state){
            QMessageBox::information(this,"重置密码","密码重置失败，请重新尝试！",QMessageBox::Yes);
            return;
        }
        QString tip = QString("密码重置成功，将重新登录！\n注意：密码被重置为[EC号]！\n");
        QMessageBox::information(this,"重置密码",tip,QMessageBox::Yes);
        modifyPassword = true;
        on_btn_exitLogin_clicked(); //退出登录
        break;
    }
    case 706:{  // 更新本地头像
        if (!state)
            return;

        QSqlQuery query;
        query.prepare("UPDATE ec_users SET usr_picture=? WHERE usr_id=?;");
        query.bindValue(0,rootObj["picture"].toInt());
        query.bindValue(1,rootObj["id"].toInt());
        if(!query.exec())
            qDebug()<<"图片存储失败！";
        break;
    }
    case 801:  // 查用户信息 √
    {
        if (!state){
            clearFriendInfo();
            return;
        }
        QJsonObject selfInfoObj = rootObj["userInfos"].toObject();
        if(selfInfoObj.isEmpty())
            return;

        // 设置头像
        int id = rootObj["id"].toInt();
        int index =  selfInfoObj["picture"].toInt();
        QString picture = getHeadPicture(0,index);
        QPixmap pixmap;
        pixmap.load(picture);
        ui->lab_headpicture->setPixmap(pixmap);
        if(id == selfId)
            ui->lab_selfPicture->setPixmap(pixmap);

        //功能限制
        if(id == selfId){
            ui->btn_deleteContact->setVisible(false);
            ui->btn_sendMsg->setVisible(false);
            ui->btn_edit->setVisible(true);
            ui->btn_editPicture->setVisible(true);
            ui->cBox_sex->setEnabled(false);
        }else{
            ui->btn_deleteContact->setVisible(true);
            ui->btn_sendMsg->setVisible(true);
            ui->btn_edit->setVisible(false);
            ui->btn_editPicture->setVisible(false);
            ui->cBox_sex->setEnabled(false);
        }
        ui->line_nickname->setText(selfInfoObj["nickname"].toString());
        ui->line_id->setText(QString::number(id));
        ui->text_selfSay->setText(selfInfoObj["self_say"].toString());
        if(selfInfoObj["sex"].toString().isEmpty())
            ui->cBox_sex->setCurrentIndex(0);
        else
            ui->cBox_sex->setCurrentText(selfInfoObj["sex"].toString());
        ui->line_phone->setText(selfInfoObj["phone"].toString());
        ui->line_region->setText(selfInfoObj["region"].toString());
        ui->line_email->setText(selfInfoObj["email"].toString());

        break;
    }
    case 802:  // 查群列表(UI初始化) √
    {
        if (!state)
            return;

        QJsonArray grpArr = rootObj["groupList"].toArray();
        if(!grpArr.isEmpty()){
            int        counts = grpArr.count();
            for (int i = 0; i < counts; i++) {
                QJsonObject subObj = grpArr.at(i).toObject();
                int groupId = subObj["groupId"].toInt();
                auto info = groups.find( groupId ).value();

                info->item->setId(subObj["groupId"].toInt());
                info->item->setNickname(subObj["nickname"].toString());
                int picture = subObj["picture"].toInt();
                info->item->setPicture(1,picture);

                db->addLocalUser(1,groupId,subObj["nickname"].toString(),picture); //添加至本地
            }
        }
        break;
    }
    case 803:  // 查群信息(展示)
    {
        if (!state)
            return;

        QJsonObject grpInfoObj = rootObj["groupInfos"].toObject();

        //1.群基本信息
        //功能限制
        int leaderId = grpInfoObj["leaderId"].toInt();
        ui->line_grpNickname->setReadOnly(true);
        ui->text_grpIntro->setReadOnly(true);
        ui->btn_grpSendMsg->setVisible(true);
        ui->line_usrGrpNickname->setReadOnly(true);
        if(leaderId == selfId){
            ui->btn_quitGroup->setText("删群");
            ui->btn_grpEdit->setVisible(true);
            ui->btn_editPicture->setVisible(true);
            ui->btn_editGrpPicture->setVisible(true);
        }else{
            ui->btn_quitGroup->setText("退群");
            ui->btn_grpEdit->setVisible(false);
            ui->btn_editGrpPicture->setVisible(false);
        }
        ui->line_grpId->setText(QString::number(grpInfoObj["groupId"].toInt()));
        ui->line_grpNickname->setText(grpInfoObj["nickname"].toString());
        ui->text_grpIntro->setText(grpInfoObj["intro"].toString());
        QString grpPicture = getHeadPicture(1,grpInfoObj["picture"].toInt());  //群头像
        QPixmap pixmap;
        pixmap.load(grpPicture);
        ui->lab_grpPicture->setPixmap(pixmap);

        //2.设置左侧信息展
        QString leaderNickname = grpInfoObj["leaderNickname"].toString();
        ui->tool_grpLeaderInfo->setText(leaderNickname);
        ui->tool_grpLeaderInfo->setToolTip(QString("账号:%1").arg(QString::number(leaderId)));
        QString leaderPicture = getHeadPicture(0,grpInfoObj["leaderPicture"].toInt()); //群主头像
        ui->tool_grpLeaderInfo->setIcon(QIcon(leaderPicture));

        QString dateTime = grpInfoObj["buildTime"].toString();
        QStringList list = dateTime.split(" ");
        ui->lab_grpBuildTime->setText(QString("于").append(list.at(0)));
        ui->lab_grpMemType->setText(QString("%1[管理] %2[成员]").arg(
                                        QString::number(grpInfoObj["adminCount"].toInt()),
                                        QString::number(grpInfoObj["memberCount"].toInt())));

        ui->line_usrGrpNickname->setText(rootObj["userGroupNickname"].toString());

        //3.设置右侧群成员信息，在此之前先清空先前数据的群成员数据
        if(groupsMems.size() > 0)
            clearGroupInfo(false); //清空先前数据的群成员数据

        QJsonArray memArr = rootObj["members"].toArray();
        QString text;
        int memberId;
        for(int i=0; i<memArr.size(); ++i){
            QJsonObject membInfo = memArr[i].toObject();
            memberId = membInfo["userId"].toInt();
            text = QString("%1[%2]").arg(membInfo["nickname"].toString(),QString::number(memberId));

            QToolButton *toolBtn = new QToolButton(this);
            toolBtn->setMinimumSize(180,28);
            toolBtn->setMaximumSize(180,28);
            toolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            toolBtn->setIconSize(QSize(30,30));
            QString picture = getHeadPicture(0,membInfo["picture"].toInt()); //群成员头像
            toolBtn->setIcon(QIcon(picture));
            toolBtn->setText(text);

            ui->vLayout_groupInfoMember->insertWidget(0,toolBtn);
            groupsMems.insert(memberId,toolBtn);
        }
        if(memArr.size() > 0){
            ui->lab_mamberTitle->setVisible(true);
            ui->scroll_groupInfoMembers->setVisible(true);
        }
        ui->stacked_contact->setCurrentIndex(groupOpPage);//完成加载再显示
        break;
    }
    case 804:  // 查询群(用于添加群聊)
    {
        if (!state) {
            ui->lab_seekTip->setText("群不存在！");
            break;
        }
        //清除上次遗留信息
        ui->lab_addTip->clear();
        ui->lab_seekTip->clear();

        bool    added = rootObj["added"].toBool();
        QJsonObject grpInfoObj = rootObj["groupInfos"].toObject();
        int index = grpInfoObj["picture"].toInt();
        QString picture = getHeadPicture(1,index);
        ui->lab_seekPicture->setPixmap(picture);

        if (added) {
            ui->btn_add->setEnabled(false);
            ui->line_addPs->setEnabled(false);
            ui->line_addPs->setText(rootObj["ps"].toString());
            QString applyTime =  rootObj["applyTime"].toString();
            QString joinTime = rootObj["joinTime"].toString();
            if(!applyTime.isEmpty()){
                QStringList list = applyTime.split(" ");
                ui->btn_add->setText("审核中...");
                ui->lab_addTip->setText(QString("申请时间：").append(list.at(0)));
            }else {
                QStringList list = joinTime.split(" ");
                ui->btn_add->setText("已添加");
                ui->lab_addTip->setText(QString("加入时间：").append(list.at(0)));
            }
        } else {
            ui->btn_add->setText("添加群");
            ui->btn_add->setEnabled(true);
            ui->line_addPs->setEnabled(true);
        }
        ui->lab_seekNickname->setText(grpInfoObj["nickname"].toString());
        ui->lab_seekType->setText("[群聊]");
        ui->lab_seekId->setText(QString::number(grpInfoObj["groupId"].toInt()));

        ui->widget_seekShow->setVisible(true);//完成加载再显示
        break;
    }
//    case 805:  // 查群成员(展示)
//    {
//        break;
//    }
    case 806:  // 查好友列表(UI初始化) √
    {
        if (!state)
            return;

        // 设置自己信息
        QJsonObject selfObj = rootObj["selfInfos"].toObject();
        auto        selfInfo = friends.find(2).value(); //拿到自己的基础控件
        selfInfo->item->setNickname(QString("[我]").append(selfObj["nickname"].toString()));
        selfInfo->item->setId(selfId);

        selfInfo->item->setPicture(0,selfObj["picture"].toInt());
        QString picture = getHeadPicture(0,selfObj["picture"].toInt());
        QPixmap pixmap;
        pixmap.load(picture);
        ui->lab_headpicture->setPixmap(pixmap); //自己的头像
        ui->lab_selfPicture->setPixmap(pixmap);

        // 设置好友信息
        QJsonArray frdArr = rootObj["friendList"].toArray();
        if(!frdArr.isEmpty()){
            int        counts = frdArr.count();
            for (int i = 0; i < counts; i++) {
                QJsonObject subObj = frdArr.at(i).toObject();
                int friendId = subObj["friendId"].toInt();
                auto info = friends.find(friendId).value();

                info->item->setNickname( subObj["nickname"].toString());
                info->item->setId(subObj["friendId"].toInt());

                //设置头像
                int picture = subObj["picture"].toInt();
                info->item->setPicture(0,picture);

                db->addLocalUser(0,friendId,subObj["nickname"].toString(),picture); //添加至本地
            }
        }
        break;
    }
    case 807:  // 查询好友列表(建群) √
    {
        if (!state)
            return;

        // 群主
        QJsonObject selfObj = rootObj["selfInfos"].toObject();
        QString picture = getHeadPicture(0,selfObj["picture"].toInt());
        ui->toolBtn_groupLeader->setIcon(QIcon(picture));

        ui->toolBtn_groupLeader->setText(QString("%1[%2]").arg(selfObj["nickname"].toString(),QString::number(selfId)));

        // 待添加群成员
        QJsonArray frdArr = rootObj["friendList"].toArray();
        if(frdArr.isEmpty())
            return;
        int        counts = frdArr.count();
        for (int i = 0; i < counts; i++) {
            if(selectGrpMems.contains(i))
                continue;
            // 1.防重复
            QJsonObject subObj = frdArr.at(i).toObject();
            int         frdId = subObj["friendId"].toInt();
            QString nickname = subObj["nickname"].toString();
            QString picture = getHeadPicture(0,subObj["picture"].toInt());

            // 2.信息缩略图
            QRadioButton *radioBtn = new QRadioButton(this);
            radioBtn->setMinimumSize(QSize(386,30));
            radioBtn->setIconSize(QSize(25,25));
            radioBtn->setAutoExclusive(false);
            radioBtn->setText(QString("%1[%2]").arg(nickname,QString::number(frdId)));
            radioBtn->setIcon((QIcon(picture)));
            ui->vLayout_addGroup->insertWidget(i,radioBtn);

            // 3.添加map映射
            selectGrpMems.insert(i,radioBtn);
        }
        break;
    }
    case 808:  // 查询好友(用于添加好友)
    {
        if (!state) {
            ui->lab_seekTip->setText("该好友不存在！");
            break;
        }
        //清除上次遗留信息
        ui->lab_seekTip->clear();
        ui->lab_addTip->clear();
        ui->lab_seekType->setText("[好友]");

        bool        added = rootObj["added"].toBool();
        QJsonObject usrInfoObj = rootObj["userInfos"].toObject();
        QString picture = getHeadPicture(0,usrInfoObj["picture"].toInt());

        QString     nickname = usrInfoObj["nickname"].toString();
        int         id = usrInfoObj["userId"].toInt();
        if (added) {
            ui->btn_add->setEnabled(false);
            ui->line_addPs->setEnabled(false);
            ui->line_addPs->setText(rootObj["ps"].toString());
            if (id == selfId) {
                ui->lab_seekType->setText("[自己]");
                ui->btn_add->setText("无法添加");;
            } else {
                QString applyTime =  rootObj["applyTime"].toString();
                QString buildTime = rootObj["buildTime"].toString();
                if(!applyTime.isEmpty()){
                    QStringList list = applyTime.split(" ");
                    ui->btn_add->setText("等待同意");
                    ui->lab_addTip->setText(QString("申请时间：").append(list.at(0)));
                }else {
                    QStringList list = buildTime.split(" ");
                    ui->btn_add->setText("已添加");
                    ui->lab_addTip->setText(QString("建立时间：").append(list.at(0)));
                }
            }
        } else {
            ui->btn_add->setText("添加好友");
            ui->btn_add->setEnabled(true);
            ui->line_addPs->setEnabled(true);
        }
        ui->lab_seekNickname->setText(nickname);
        ui->lab_seekId->setText(QString::number(id));
        ui->lab_seekPicture->setPixmap(picture);

        ui->widget_seekShow->setVisible(true);//加载完成数据之后再显示
        break;
    }
    case 809:{ // 查询好友详情信息(展示)
        break;
    }
    case 810: // UI控件初始化 ---1次√
    {
        if (!state)
            return;
        int count = 0;

        //聊天对象控件
        QJsonArray chatArr = rootObj["chatObjectList"].toArray();
//        qDebug()<<chatArr;
        count = chatArr.size();
        for (int i = 0; i < count; ++i) {
          ChatObjInfo *info = new ChatObjInfo;
          info->uiChat = new UiChat(this);
          info->item = new ItemChatObject(this);
          chatObjects.insert(chatArr[i].toInt(),info);

          ui->vLayout_message->insertWidget(i, info->item);  //顺序插入
          ui->stacked_message->insertWidget(i, info->uiChat);  //栈容器中
          connectChatObject(info->item,info->uiChat); //连接信号
        }

        //好友列表控件
        QJsonArray frdArr = rootObj["friendList"].toArray();
        count = frdArr.size();
        for (int i = 0; i < count; ++i) {
            ContactInfo *info = new ContactInfo;
            info->item = new ItemContact(this);
            friends.insert(frdArr[i].toInt(),info);
            ui->vLayout_friend->insertWidget(i + 3, info->item); //前面已存在三个基础控件
            connect(info->item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::Friend,info->item); });
        }

        //群聊控件
        QJsonArray grpArr = rootObj["groupList"].toArray();
        count = grpArr.size();
        for (int i = 0; i < count; ++i) {
            ContactInfo *info = new ContactInfo;
            info->item = new ItemContact(this);
            groups.insert(grpArr[i].toInt(),info);
            ui->vLayout_group->insertWidget(i + 1, info->item);  //顺序插入
            connect(info->item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::Group,info->item); }); //连接信号
        }

        //控件初始化完成，请求数据初始化
        emit resourceRequest(811, 0, 0,"");  //请求聊天对象信息去初始化控件
        emit resourceRequest(802, 0, 0,"");  // 拿到群聊列表去初始化控件
        emit resourceRequest(806, 0, 0,"");  // 拿到好友列表去初始化控件
        emit resourceRequest(902, 0, 0,"");  // 拿到好友申请列表
        break;
    }
    case 811:  // 查询聊天对象-好友&群(UI初始化)√
    {
        if (!state )
            return;

        QJsonArray chatObjArr = rootObj["chatObjectList"].toArray();
        //没有聊天对象
        if(chatObjArr.isEmpty()){
            return;
        }

        int        counts = chatObjArr.count();
        for (int i = 0; i < counts; i++) {
//            qDebug()<<"counts="<<counts;
            QJsonObject subObj = chatObjArr.at(i).toObject();
            ChatObjInfo *info = nullptr;

            //群聊和私聊区别显示,0 1
            int     type = subObj["type"].toInt();
            int sideId;
            if(type){
                sideId = subObj["groupId"].toInt();
                info = chatObjects.find(sideId).value();
                info->isGrpId = true;
                info->item->setOnlineStateVisible(false);
                info->uiChat->setSideType(type);
            }else{
                sideId = subObj["friendId"].toInt();
                info = chatObjects.find(sideId).value();
                info->isFrdId = true;
                info->item->setOnlineState(subObj["onlineState"].toBool());
                info->uiChat->setSidebarVisible(false);
                info->uiChat->setSideType(type);
            }
            int picture = subObj["picture"].toInt();
            info->item->setPicture(type,picture); //头像

            info->item->setNickname(subObj["nickname"].toString());
            info->item->setRecentChatTime(subObj["recentChatTime"].toString());
            info->uiChat->setSideNickname(subObj["nickname"].toString(),type);

            //初始化群聊中的群成员列表
            if(type){
                QJsonArray memArr = subObj["memberInfos"].toArray();
                int nums = memArr.size();
//                qDebug()<<"nums="<<nums;
                for (int i = 0; i < nums; ++i) {
                    QJsonObject memInfo = memArr[i].toObject();
                    //第一个始终为群主
                    QString tip;
                    if(0 == i){
                        tip = QString("昵称:%1\n账户:%2").arg(memInfo["nickname"].toString(),
                                             QString::number(memInfo["userId"].toInt()));
                        info->uiChat->addGroupMembers(memInfo["picture"].toInt(), "群主",tip);
                    }else{
                        tip = QString("昵称:%1\n账户:%2").arg(memInfo["nickname"].toString(),
                                             QString::number(memInfo["userId"].toInt()));
                        info->uiChat->addGroupMembers(memInfo["picture"].toInt(), "成员",tip);
                    }
                    info->uiChat->setMemberType(subObj["adminCount"].toInt(),subObj["memberCount"].toInt());//注意：adminCount=1 !
                }
            }
            initLocalMsg(info,sideId); //加载本地聊天记录
        }

        emit resourceRequest(812, 0, 0,""); //请求离线消息

        break;
    }
    case 812:  //查询离线消息(用于初始化)
    {
        if(!state)
            return;
        QJsonArray chatMsgList = rootObj["chatMsgList"].toArray();
        if(chatMsgList.isEmpty())
            return;

        for (int i = 0; i < chatMsgList.size(); ++i) {
            QJsonObject chatMsgObj = chatMsgList[i].toObject();
            int type = chatMsgObj["type"].toInt();

            //拿到对应界面去初始化数据
            int id;
            if(!type)
                id = chatMsgObj["friendId"].toInt();
            else
                id = chatMsgObj["groupId"].toInt();

            //去现有的聊天对象中查找是否已存在，不存在就新创建一个
            auto it = chatObjects.find(id);
            ChatObjInfo* info;
            if(it == chatObjects.end())
                info = createChatObject(id,type);
            else
                info = it.value();

            info->uiChat->recvChatMsg(type,chatMsgObj);
            info->item->setRecentUnread(chatMsgObj["unread"].toInt());
            QString message = chatMsgObj["recentMsg"].toString();
            if(messageHasImage(message))
                info->item->setRecentChatMsg("图片消息");
            else{
                if(message.length() > 20)
                    info->item->setRecentChatMsg(message.left(20));
                else
                    info->item->setRecentChatMsg(message);
            }
        }
        emit resourceRequest(606,0,0,"");  //服务器不在需要保留离线消息

        break;
    }
    case 813:  // 查聊天文件
    {
        break;
    }
    case 814: // 查用户配置
    {
        break;
    }
    case 815: // 好友在线状态
    {
        if(!state)
            return;

        QJsonArray friendArr = rootObj["onlineStateList"].toArray();
        QJsonObject singleObj;
        for (int i = 0; i < friendArr.size(); ++i) {
            singleObj = friendArr[i].toObject();
            auto it = chatObjects.find(singleObj["friendId"].toInt());
            // 判断一下，避免数据已回来但用户把某个聊天对象删除了
            if(it != chatObjects.end()) {
                it.value()->item->setOnlineState( singleObj["onlineState"].toBool() );
            }
//            qDebug()<<singleObj["friendId"].toInt();
        }
        break;
    }
    case 816: // 查询所有好友/群聊昵称和头像
    {
        if (!state)
            return;
//        qDebug()<<json.data();
        QJsonArray friendList = rootObj["friendList"].toArray();
        for(int i=0; i<friendList.size(); ++i){
            QJsonObject obj = friendList[i].toObject();
            int friendId = obj["friendId"].toInt();
            QString nickname = obj["nickname"].toString();
            int picture = obj["picture"].toInt();
            db->updateUserInfo(friendId,0,nickname,picture);

            //更新聊天对象信息
           auto it = chatObjects.find(friendId);
           if(it != chatObjects.end()){
               it.value()->item->setNickname(nickname);
               it.value()->item->setPicture(0,picture);
           }
        }
        QJsonArray groupList = rootObj["groupList"].toArray();
        for(int j=0; j<groupList.size(); ++j){
            QJsonObject obj = groupList[j].toObject();
            int groupId = obj["groupId"].toInt();
            QString groupName = obj["nickname"].toString();
            int groupPicture = obj["picture"].toInt();
            db->updateUserInfo(groupId,1,groupName,groupPicture); //更新本地数据

            //更新聊天对象信息
            auto it = chatObjects.find(groupId);
            if(it != chatObjects.end()){
               it.value()->item->setNickname(groupName);
               it.value()->item->setPicture(1,groupPicture);
            }
        }

        break;
    }
    case 901:  // 申请加好友/群状态回应
    {
        if (state){
            ui->btn_add->setEnabled(false);
            ui->line_addPs->setEnabled(false);
            int type = rootObj["type"].toInt(); //0-好友  1-群聊
            if (type)
                ui->btn_add->setText("等待审核");
            else
                ui->btn_add->setText("等待同意");

            QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
            ui->lab_addTip->setText(QString("申请时间：").append(date));
        }else{
            ui->lab_addTip->clear();
            ui->lab_seekTip->setText("申请失败，请检查网络配置！");
        }
        break;
    }
    case 902:  // 申请列表
    {
        if (!state)
            return;

        QJsonArray subArr = rootObj["applicantList"].toArray();
        int counts = subArr.size();

        //设置提示
        auto it = friends.find(0);
        it.value()->item->setApplicants(counts);

        for (int i = 0; i < counts; ++i) {
            if(applicants.contains(i))
                continue;

            newMessageTip(0); //消息提示声音
            ItemFriendApply *item = new ItemFriendApply(this);
            QJsonObject subObj =  subArr[i].toObject();

            QString ps = subObj["ps"].toString();
            int type = subObj["type"].toInt();// 0-好友  1-群
            item->setNickname(subObj["nickname"].toString(),subObj["applicantId"].toInt(),type);
            if(type)
                item->setId(subObj["groupId"].toInt(),type);
            else
                item->setId(subObj["applicantId"].toInt(),type);

            item->setPs(ps);

            //设置头像
            int picture = subObj["picture"].toInt();
            item->setPicture(type,picture);

            ui->vLayout_friendApply->insertWidget(i,item);
            applicants.insert(i,item);

            connect(item,&ItemFriendApply::agree,this,[=]{
                QJsonObject obj;
                obj.insert("agree",true);
                if(item->getType())
                    emit resourceRequest(502,item->getApplicantId(),item->getGroupId(),obj);
                else
                    emit resourceRequest(501,item->getApplicantId(),0,obj);
            });
            connect(item,&ItemFriendApply::refuse,this,[=]{
                QJsonObject obj;
                obj.insert("agree",false);
                if(item->getType())
                    emit resourceRequest(502,item->getApplicantId(),item->getGroupId(),obj);
                else
                    emit resourceRequest(501,item->getApplicantId(),0,obj);
            });
        }
        break;
    }
    case 903:{ //申请回应
        int type =  rootObj["type"].toInt();
        // 群聊
        if(type){
            int groupId = rootObj["groupId"].toInt();
            QJsonObject subObj = rootObj["groupInfo"].toObject();

            // 创建群聊
            ContactInfo *info = new ContactInfo;
            info->item = new ItemContact(this);
            info->item->setNickname(subObj["nickname"].toString());
            info->item->setPicture(1,subObj["picture"].toInt());
            info->item->setId(groupId);
            groups.insert(groupId,info);
            ui->vLayout_group->insertWidget(1, info->item);
            connect(info->item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::Group,info->item); }); //连接信号

            //添加到本地
            db->addLocalUser(1,groupId,subObj["nickname"].toString(),subObj["picture"].toInt());
        }else { //好友
            int sideId;
            QJsonObject subObj;
            int identify = rootObj["identify"].toInt();
            if(!identify){
                sideId = rootObj["receiver"].toInt();
                subObj = rootObj["receiverInfo"].toObject();
            }else {
                sideId = rootObj["applicant"].toInt();
                subObj = rootObj["applicantInfo"].toObject();
            }
            // 创建好友
            ContactInfo *info = new ContactInfo;
            info->item = new ItemContact(this);
            info->item->setNickname(subObj["nickname"].toString());
            info->item->setPicture(0,subObj["picture"].toInt());
            info->item->setId(sideId);
            friends.insert(sideId,info);
//            int index = friends.size();
            ui->vLayout_friend->insertWidget(3, info->item); //追加到末尾
            connect(info->item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::Friend,info->item); });

            //添加到本地
            db->addLocalUser(0,sideId,subObj["nickname"].toString(),subObj["picture"].toInt());
        }

        break;
    }
    default:
        break;
    }
}

/***********************************************************************
 * @brief: 聊天对象信号连接
 * @param: ItemChatObject* item,UiChat* uiChat
 * @note:
 ***********************************************************************/
void UiHome::connectChatObject(ItemChatObject* item,UiChat* uiChat)
{
     // 删除缩略图&聊天界面
    connect(item, &ItemChatObject::removeCliked, this, [=] {
        for (auto it = chatObjects.begin(); it != chatObjects.end(); ++it) {
            if (item == it.value()->item) {
                if(it.value()->isFrdId)
                    emit resourceRequest(605,it.key(),0,"");
                else
                    emit resourceRequest(605,0,it.key(),"");

                chatObjects.remove(it.key());
                delete it.value();
                it.value() = nullptr;
                break;
            }
        }
    });
    connect(uiChat, &UiChat::sendChatMsg, this, &UiHome::on_sendChatMsg);    // 发送信息
    connect(uiChat, &UiChat::sendChatFile, this, &UiHome::on_sendChatFile);  // 发送文件
    connect(this, &UiHome::uiChatMax, uiChat, &UiChat::on_uiChatMax);
    connect(item, &ItemChatObject::clicked, uiChat, [=] {
        for (auto it = chatObjects.begin(); it != chatObjects.end(); ++it) {
            if (item == it.value()->item) {
                // 展示对应的聊天窗口
                ui->stacked_message->setCurrentWidget(it.value()->uiChat);
                // 设置输入框为焦点
                it.value()->uiChat->setInputFocus();
                break;
            }
        }
        buttonExclusive(ButtonType::ChatObject,item);
    });
}


/***********************************************************************
 * @brief: 联系人点击事件处理
 * @param:
 * @note:
 ***********************************************************************/
void UiHome::contactCliked(ClikedType clikedType, ItemContact *item)
{
    if(clikedType != ClikedType::Self && editingSelfInfo){
        QMessageBox::information(this,"信息修改","[自己]请先保存编辑信息！",QMessageBox::Ok);
        return;
    }
    switch (clikedType) {
     case ClikedType::FriendApply:{
        for (auto it = friends.begin(); it != friends.end(); ++it) {
            if (item == it.value()->item && !it.value()->isSelect) {
                emit resourceRequest(902, 0, 0,"");
                ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_FRIEND_APPLY);
                friendOpPage = PAGE_CONTACT_OP_FRIEND_APPLY;
                buttonExclusive(ButtonType::Friend,item);
                break;
            }
        }
        break;
    }
    case ClikedType::AddContact:{
        closeHeadPicture();// 用于头像选择框关闭
        for (auto it = friends.begin(); it != friends.end(); ++it) {
            if (item == it.value()->item && !it.value()->isSelect) {

                clearSeekInfo(); //清除上次遗留信息

                ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_ADD_CONTACT);
                friendOpPage = PAGE_CONTACT_OP_ADD_CONTACT;

                ui->btn_seekFriend->setChecked(true);
                ui->line_seekInput->setFocus(); //设置搜索框焦点

                buttonExclusive(ButtonType::Friend,item);
            }
        }
        break;
    }
    case ClikedType::Self:{
        for (auto it = friends.begin(); it != friends.end(); ++it) {
            if (item == it.value()->item && !it.value()->isSelect) {
                emit resourceRequest(801, 0, 0,"");
                ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_FRIEND_INFO);
                friendOpPage = PAGE_CONTACT_OP_FRIEND_INFO;

                //允许编辑信息
                ui->btn_sendMsg->setVisible(false);
                ui->btn_edit->setVisible(true);

                buttonExclusive(ButtonType::Friend,item);
                break;
            }
        }
        break;
    }
    case ClikedType::AddGroup:{
        closeHeadPicture();// 用于头像选择框关闭
        if(currChatObjInfo != nullptr)
            currChatObjInfo->uiChat->closeEmojiShow();//关闭表情选择框

        for (auto it = groups.begin(); it != groups.end(); ++it) {
            if (item == it.value()->item) {
                on_btn_cancelAddGroup_clicked(); //清除信息
                emit resourceRequest(807, 0, 0,"");
            }
            // 只有没被点击才去执行对应操作
            if(!it.value()->isSelect) {
                ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_CREATE_GROUP);
                groupOpPage = PAGE_CONTACT_OP_CREATE_GROUP;
                buttonExclusive(ButtonType::Group,item);
                break;
            }
        }
        break;
    }
    case ClikedType::Friend:{
        for (auto it = friends.begin(); it != friends.end(); ++it) {
            if (item == it.value()->item && !it.value()->isSelect) {

//                clearFriendInfo(); //清空先前数据
                ui->btn_editPicture->setVisible(false);

                emit resourceRequest(801, item->getContactId(), 0,"");
                ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_FRIEND_INFO);
                friendOpPage = PAGE_CONTACT_OP_FRIEND_INFO;
                currContactFrdID = it.key();//记录当前好友ID
                buttonExclusive(ButtonType::Friend,item);
                break;
            }
        }
        break;
    }
    case ClikedType::Group:{
        for (auto it = groups.begin(); it != groups.end(); ++it) {
            if (item == it.value()->item && !it.value()->isSelect) {

                emit resourceRequest(803, 0, item->getContactId(),""); //请求群消息

                ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_GROUP_INFO);
                groupOpPage = PAGE_CONTACT_OP_GROUP_INFO;
                currContactGrpID = it.key(); //记录当前群ID
                buttonExclusive(ButtonType::Group,item);
                break;
            }
        }
        break;
    }
    default:
        break;
    }
}

/***********************************************************************
 * @brief: 自定义控件按钮互斥操作
 * @param: ButtonType buttonType, void *item
 * @note: buttonType类型不可弄错！
 ***********************************************************************/
void UiHome::buttonExclusive(ButtonType buttonType, void *item)
{
    switch (buttonType) {
    case ButtonType::ChatObject:{
        ItemChatObject *_item = static_cast<ItemChatObject*>(item);
        for (auto it = chatObjects.begin(); it != chatObjects.end(); ++it) {
            if (it.value()->isSelect) {
                it.value()->isSelect = false;
                it.value()->item->setBackground(false); //恢复
                it.value()->uiChat->closeEmojiShow(); //关闭表情包显示
            }
            if (_item == it.value()->item) {
                it.value()->isSelect = true;
                it.value()->item->clearRecentUnread();
                _item->setBackground(true);
                currChatObjectID = it.key();//记录当前对象ID
                currChatObjInfo = it.value(); //记录当前聊天对象
            }
        }
        break;
    }
    case ButtonType::Friend:{
        ui->lab_selfSayAndIntro->setText("标签：");
        ItemContact *_item = static_cast<ItemContact*>(item);
        for (auto it = friends.begin(); it != friends.end(); ++it) {
            if (it.value()->isSelect) {
                it.value()->isSelect = false;
                it.value()->item->setBackground(false);
            }
            if (_item == it.value()->item) {
                it.value()->isSelect = true;
                _item->setBackground(true);
            }
        }
        break;
    }
    case ButtonType::Group:{
        ui->lab_selfSayAndIntro->setText("简介：");
        ItemContact *_item = static_cast<ItemContact*>(item);
        for (auto it = groups.begin(); it != groups.end(); ++it) {
            if (it.value()->isSelect) {
                it.value()->isSelect = false;
                it.value()->item->setBackground(false);
            }
            if (_item == it.value()->item) {
                it.value()->isSelect = true;
                _item->setBackground(true);
            }
        }
        break;
    }
    default:
        break;
    }
}

//只有所有，好友和群聊没分！群聊也没实现!
void UiHome::initFileList()
{
    QString sql,subSql;
    QSqlQuery query,subQuery;

    sql = QString("SELECT COUNT(sender)  "
                  "FROM (SELECT * FROM ec_messages WHERE frd_id=%1 AND msg_type='file'); "
                  ).arg(QString::number(selfId));
    int start = 0,end = 0;
    if(query.exec(sql) && query.next())
        end = query.value(0).toInt();
    if(end > MSG_COUNT)
        start = end - MSG_COUNT;
    //正式开始拿数据
    sql = QString("SELECT * FROM ec_messages WHERE frd_id=%1 AND msg_type='file'  "
                  "ORDER BY send_time LIMIT %2,%3; " //DESC
                  ).arg(QString::number(selfId),QString::number(start),QString::number(end));
    if(query.exec(sql)){
        int sender=0,receiver=0;
        while (query.next()) {
            QString fileName = query.value("file_name").toString();
            QString dateTime = query.value("send_time").toDateTime().toString("yyyy年MM月dd日");

            QString text = QString("%1  [日期:%2]").arg(fileName,dateTime);
//            QString filePath = query.value("file_path").toString();
            QString filePath = QString("%1/%2").arg(query.value("file_path").toString(),fileName);

            //拿到发送者和接收者名称
            sender = query.value("sender").toInt();
            receiver = query.value("frd_id").toInt();
            QString tip;
            if(sender == selfId){
                subSql = QString("SELECT usr_nickname FROM ec_users WHERE usr_id=%1;").arg(receiver);
                if(subQuery.exec(subSql) && subQuery.next()){
                    tip = QString("[自己]发送者：%1(%2)\n[好友]接收者：%3(%4)").arg(selfNickname,
                                                                        QString::number(sender),
                                                                        subQuery.value(0).toString(),
                                                                        QString::number(receiver));
                }

            }else{
                subSql = QString("SELECT usr_nickname FROM ec_users WHERE usr_id=%1;").arg(sender);
                if(subQuery.exec(subSql) && subQuery.next()){
                    tip = QString("[好友]发送者：%1(%2)\n[自己]接收者：%3(%4)").arg(subQuery.value(0).toString(),
                                                                        QString::number(sender),
                                                                        selfNickname,
                                                                        QString::number(receiver));
                }
            }
            createFileToolButton(fileName,text,filePath,tip);
        }
    }
}

void UiHome::initBaseContact()
{
    //基础控件
    for (int i = 0; i < 3; i++) {
        ItemContact *item = new ItemContact(this);
        ContactInfo *info = new ContactInfo;
        info->item = item;
        switch (i) {
        case 0: {  // top-1 好友申请处理
            item->setPicture(0,1001);
            item->setNickname("好友申请");
            item->setTipStyle();

            friends.insert(i, info);
            ui->vLayout_friend->insertWidget(i, item);
            connect(item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::FriendApply,item); });
            break;
        }
        case 1: {  // top-2 添加好友
            item->setPicture(0,1002);
            item->setNickname("添加好友");

            friends.insert(i, info);
            ui->vLayout_friend->insertWidget(i, item);
            connect(item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::AddContact,item); });
            break;
        }
        case 2: {  // top-3 个人信息
            friends.insert(i, info);
            ui->vLayout_friend->insertWidget(i, item);
            connect(item, &ItemContact::clicked, this, [=] { contactCliked(ClikedType::Self,item); });
            break;
        }
        default:
            break;
        }
    }

    //创建群聊
    ItemContact *item = new ItemContact(this);
    ContactInfo *info = new ContactInfo;
    info->item = item;
    item->setPicture(1,1003);
    item->setNickname("创建群聊");
    groups.insert(0,info);
    ui->vLayout_group->insertWidget(0,item);
    connect(item,&ItemContact::clicked,this,[=]{ contactCliked(ClikedType::AddGroup,item); });
}

void UiHome::createFileToolButton(const QString &fileName,const QString &text, const QString &filePath, const QString &tip)
{
    auto it = files.find(fileName);
    if(it != files.end())
        return;
    QToolButton *toolBtn = new QToolButton(this);
    toolBtn->setMinimumSize(675,40);
    toolBtn->setMaximumSize(675,40);
    toolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBtn->setIconSize(QSize(30,30));
    toolBtn->setIcon(QIcon(":/img/file.png"));
    toolBtn->setText(text);
    toolBtn->setToolTip(tip);

    //打开文件
    connect(toolBtn,&QToolButton::clicked,this,[=]{
//        qDebug()<<filePath;
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
//        QProcess process;
//        process.startDetached(filePath,QStringList());
    });
    files.insert(fileName,toolBtn);
    ui->vLayout_allFile->insertWidget(0,toolBtn);
}

//创建一个新的聊天对象
ChatObjInfo* UiHome::createChatObject(const int &id,const int &type)
{
    ChatObjInfo *info = new ChatObjInfo;
    info->item = new ItemChatObject(this);
    info->uiChat = new UiChat(this);

    QString sql = QString("SELECT usr_nickname,usr_picture FROM ec_users WHERE usr_id=%1;").arg(QString::number(id));
    QSqlQuery query;
    if(query.exec(sql) && query.next()){
        info->uiChat->setSideNickname(query.value("usr_nickname").toString(),GROUP);
        info->item->setNickname(query.value("usr_nickname").toString());
        info->item->setPicture(type,query.value("usr_picture").toInt());
    }
    info->item->setRecentChatTime(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    //群聊和好友的不同处
    if(type){
        info->isGrpId = true;
        emit resourceRequest(505,0,id,""); //初始化一下群成员信息
    }else{//好友
        info->uiChat->setSidebarVisible(false);
        info->isFrdId = true;
    }
    connectChatObject(info->item,info->uiChat);//信号连接
    ui->stacked_message->addWidget(info->uiChat);
    chatObjects.insert(id,info);//map添加
    ui->vLayout_message->insertWidget(0,info->item);//新的对象在最前面(置顶先排除)

    return info;
}

void UiHome::clearSeekInfo()
{
    ui->lab_seekTip->clear();
    ui->line_seekInput->clear();
    ui->line_addPs->clear();
    ui->widget_seekShow->setVisible(false);
}

void UiHome::clearFriendInfo()
{
    ui->line_id->clear();
    ui->line_nickname->clear();
    ui->text_selfSay->clear();
    ui->cBox_sex->setCurrentIndex(0);
    ui->line_phone->clear();
    ui->line_region->clear();
    ui->line_email->clear();
    ui->lab_headpicture->clear();
}

void UiHome::clearGroupInfo(bool isAll)
{
    if(isAll){
        ui->line_grpNickname->clear();
        ui->line_groupName->clear();
        ui->line_grpId->clear();
        ui->text_grpIntro->clear();
        ui->tool_grpLeaderInfo->setIcon(QIcon(""));
        ui->tool_grpLeaderInfo->setText("");
        ui->lab_grpBuildTime->clear();
        ui->lab_grpMemType->clear();
        ui->line_usrGrpNickname->clear();
        ui->lab_grpPicture->clear();
    }
    ui->lab_mamberTitle->setVisible(false);
    ui->scroll_groupInfoMembers->setVisible(false);
    //删除群成员
    if(groupsMems.size() <= 0)
        return;

    for(auto it=groupsMems.begin(); it!=groupsMems.end(); ++it){
        auto toolBtn = it.value();
        ui->vLayout_groupInfoMember->removeWidget(toolBtn);
    }
    // 当T的类型为指针时，调用clear方法能置空，但并不能释放其内存。
    // qDeleteAll可以释放容器元素内存，但没有对容器的置空操作，也就是size没变。
    // 所以qDeleteAll之后必须加上clear方法。
    qDeleteAll(groupsMems);
    groupsMems.clear();
}

void UiHome::setInputRegExp()
{
    //搜索EC号/手机号码：纯数字
    QRegExp numReg(NUM_REG);
    ui->line_seekInput->setValidator(new QRegExpValidator(numReg,this));
    ui->line_phone->setValidator(new QRegExpValidator(numReg,this));

    //密码：英文大小写+数字
    QRegExp pwdReg(PWD_REG);
    ui->line_newPassword->setValidator(new QRegExpValidator(pwdReg,this));
    ui->line_sureNewPassword->setValidator(new QRegExpValidator(pwdReg,this));

    //邮箱
    QRegExp emailReg(EMAIL_REG);
    ui->line_email->setValidator(new QRegExpValidator(emailReg,this));

    //地区：英文+汉字+数字
    QRegExp classReg(ZH_EN_NUM_REG);
    ui->line_region->setValidator(new QRegExpValidator(classReg,this));
//    ui->line_nickname->clear();
//    ui->text_selfSay->clear();

}

void UiHome::clearCreateGroupInfo()
{
    ui->lab_groupPicture->setPixmap(QPixmap(":/img/head/group.png"));
    ui->line_groupName->clear();

    //创建群聊的好友选择列表
    for(auto it=groups.begin();it!=groups.end();++it)
        ui->vLayout_addGroup->removeWidget(it.value()->item);
    qDeleteAll(selectGrpMems);
    selectGrpMems.clear();
}

void UiHome::clearFiles()
{
    //5.文件列表
    for(auto it=files.begin();it!=files.end();++it)
        ui->vLayout_allFile->removeWidget(it.value());
    qDeleteAll(files);
    files.clear();
}

void UiHome::clearModifyPassword()
{
    ui->line_newPassword->clear();
    ui->line_sureNewPassword->clear();
    ui->lab_passwordModifyTip->clear();
}

void UiHome::clearAllData()
{
    //1.聊天对象
   for(auto it=chatObjects.begin();it!=chatObjects.end();++it){
       if(it.value() == currChatObjInfo)
           currChatObjInfo = nullptr; //当前聊天对象
       ui->vLayout_message->removeWidget(it.value()->item);
       ui->stacked_message->removeWidget(it.value()->uiChat);
   }
   qDeleteAll(chatObjects);
   chatObjects.clear();

   //2.申请人列表
   for(auto it=applicants.begin();it!=applicants.end();++it)
       ui->vLayout_friendApply->removeWidget(it.value());
   qDeleteAll(applicants);
   applicants.clear();

   //3.好友
   for(auto it=friends.begin();it!=friends.end();++it)
       ui->vLayout_friend->removeWidget(it.value()->item);
   qDeleteAll(friends);
   friends.clear();

   //4.群聊列表
   for(auto it=groups.begin();it!=groups.end();++it)
       ui->vLayout_group->removeWidget(it.value()->item);
   qDeleteAll(groups);
   groups.clear();

   clearFiles(); //清除文件消息

   clearCreateGroupInfo();
   clearSeekInfo();
   clearFriendInfo();
   clearGroupInfo();
   clearModifyPassword();

   initBaseContact(); //初始化基础控件
   dockIconChanged(0);//恢复消息按钮被点击
   ui->lab_suggestTip->clear();
   ui->line_newPassword->clear();
   ui->line_sureNewPassword->clear();

   // 初始默认页
   ui->stacked_home->setCurrentIndex(PAGE_HOME_MSG);
   lastDockIndex = PAGE_HOME_MSG;
   ui->stacked_message->setCurrentIndex(PAGE_HOME_MSG_DEFAULT);
   ui->stacked_contact->setCurrentIndex(PAGE_CONTACT_OP_DEFAULT);
   ui->stacked_contactShow->setCurrentIndex(PAGE_CONTACT_TYPE_FRIEND);
   ui->stackedWidget_file->setCurrentIndex(0);
   ui->btn_friend->setChecked(true);
   contactType = PAGE_CONTACT_TYPE_FRIEND;

   //global中的全局变量
   lastId = selfId; //记录当前用户id
   selfId = 0;
   selfNickname = "";

   //uireply中的全局变量
   lastDockIndex = 0; //用于dock栏点击&恢复显示
   currChatObjectID = 0;
   contactType = 0;//0-好友界面   1-群聊界面
   friendOpPage = 0; // 1  2  4
   groupOpPage = 0;// 3  5
   currContactFrdID = 0;  // 2.3联系人页面项对应的EC号
   currContactGrpID = 0;
}

void UiHome::closeHeadPicture()
{
    if(selectingPicture){
        headPicture->hide();
        selectingPicture = false;
    }
}

void UiHome::deleteFriend(const int &friendId)
{
    if(friendId <=3 )
        return;

    auto it = friends.find(friendId);
    if(it != friends.end()){
        friends.remove(friendId);
        ui->vLayout_friend->removeWidget(it.value()->item);

        //聊天列表是否存在改好友
        auto _it = chatObjects.find(friendId);
        if(_it != chatObjects.end()){
            chatObjects.remove(friendId);
            ui->vLayout_message->removeWidget(_it.value()->item);
            ui->stacked_message->removeWidget(_it.value()->uiChat);
            delete _it.value();
        }

        delete it.value();
        clearFriendInfo();
    }
}

void UiHome::quitGroup(const int &groupId)
{
    if(groupId < 2)
        return;
    auto it = groups.find(groupId);
    if(it != groups.end()){
        friends.remove(groupId);
        ui->vLayout_group->removeWidget(it.value()->item);

        //聊天列表是否存在改群聊
        auto _it = chatObjects.find(groupId);
        if(_it != chatObjects.end()){
            chatObjects.remove(groupId);
            ui->vLayout_message->removeWidget(_it.value()->item);
            ui->stacked_message->removeWidget(_it.value()->uiChat);
            delete _it.value();
        }
        delete it.value();
        clearGroupInfo();
    }
}

QString UiHome::getHeadPicture(const int type,int picture)
{
    QString _picture;
    // 群聊
    if(picture <= 0){
        if(type)
            _picture = ":/img/head/group.png"; //默认头像
        else
            _picture = ":/img/head/person.png"; //默认头像
    }else{
        _picture = QString(":/img/head/%1.png").arg(QString::number(picture));
    }
    return _picture;
}

void UiHome::newMessageTip(const int &type)
{
    // 消息提示是否开启
    if(file->getNewNotifyVoice()){
        // 群消息通知是否开启
        if(type){
            if(file->getGroupNotifyVoice())
                QSound::play(":/audio/simpeCuteAlert.wav");
        } else{
            QSound::play(":/audio/simpeCuteAlert.wav");
        }
    }
    this->activateWindow(); //设置窗口为活动窗口
}

 //检测消失是否还有图片
bool UiHome::messageHasImage(const QString &msg)
{
    QRegularExpression re(R"(<img[^>]*src=[\"']data:image/(png|jpg|jpeg);base64,([^"]*)[\"'][^>]*>)",
                          QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(msg);
    if (match.hasMatch()) {
        return true;
    }
    return false;
}

void UiHome::initLocalMsg(const ChatObjInfo *info,const int& chatObjId)
{
    QString sql,subSql;
    QSqlQuery query,subQuery;
    QJsonObject chatMsg;
    int type = 0;
    if(info->isFrdId){
        sql = QString("SELECT COUNT(sender) FROM ( "
                      "SELECT * FROM ec_messages WHERE sender=%1 AND frd_id=%2   "
                      "UNION  "
                      "SELECT * FROM ec_messages WHERE sender=%2 AND frd_id=%1 ); "
                      ).arg(QString::number(selfId), QString::number(chatObjId));
        int start = 0,end = 0;
        if(query.exec(sql) && query.next())
            end = query.value(0).toInt();
        if(end > MSG_COUNT)
            start = end - MSG_COUNT;
        //正式开始拿数据
        sql = QString("SELECT * FROM ec_messages WHERE sender=%1 AND frd_id=%2 AND type=0  "
                      "UNION "
                      "SELECT * FROM ec_messages WHERE sender=%2 AND frd_id=%1 AND type=0  "
                      "ORDER BY send_time LIMIT %3,%4; "
                      ).arg(QString::number(selfId), QString::number(chatObjId),QString::number(start),QString::number(end));
    }else{
        type = 1;
        sql = QString("SELECT COUNT(sender) FROM ec_messages WHERE grp_id=%1 AND type=1;").arg(QString::number(chatObjId));
        int start = 0,end = 0;
        if(query.exec(sql) && query.next())
            end = query.value(0).toInt();
        if(end > MSG_COUNT)
            start = end - MSG_COUNT;
        //正式开始拿数据
        sql = QString("SELECT * FROM ec_messages WHERE grp_id=%1 AND type=%2 "
                      "ORDER BY send_time LIMIT %3,%4;"
                      ).arg(QString::number(chatObjId),QString::number(type),QString::number(start),QString::number(end));
    }
//        qDebug()<<sql;
    if(query.exec(sql)){
        while (query.next()) {
            QString msgType = query.value("msg_type").toString();
            int sender = query.value("sender").toInt();
            chatMsg["message"] = query.value("message").toString();
            chatMsg["msgType"] = msgType;
            chatMsg["sendTime"] = query.value("send_time").toString();
            chatMsg["sender"] = sender;
            int picture = query.value("senderPicture").toInt();
            chatMsg["type"] = type;

            QString nickname;
            if(sender == selfId){
                nickname = selfNickname;
            }else{
                subSql = QString("SELECT usr_nickname FROM ec_users WHERE usr_id=%1;").arg(QString::number(sender));
                if(subQuery.exec(subSql) && subQuery.next())
                    nickname = subQuery.value("usr_nickname").toString();
            }

            if(type){ //群
                chatMsg["senderInfo"];
                QJsonObject suObj;
                suObj["nickname"] = nickname;
                suObj["picture"] = picture;
                chatMsg["senderInfo"] = suObj;
            }else{
                chatMsg["nickname"] = nickname;
                chatMsg["picture"] = picture;
            }

            if ("text" != msgType){
                chatMsg["fileName"] = query.value("file_name").toString(); // 不做文件消息的非空判断了，是文件就必须得存这些信息！
                chatMsg["suffix"] = query.value("suffix").toString();
                chatMsg["filePath"] =query.value("file_path").toString();
            }

            info->uiChat->localChatMsg(type,chatMsg); //显示消息
//                qDebug()<<chatMsg;
        }
        if(query.last()){ //设置最近聊天消息
            QString message = query.value("message").toString();
            if(messageHasImage(message))
                info->item->setRecentChatMsg("图片消息");
            else{
                if(message.length() > 20)
                    info->item->setRecentChatMsg(message.left(20));
                else
                    info->item->setRecentChatMsg(message);
            }
            info->item->setRecentChatTime(query.value("send_time").toString());
        }
    }//sql执行成功
}

//文件管理
void UiHome::on_btn_openFilePos_clicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(FILE_PATH(selfId)) );}
void UiHome::on_btn_openScreenShotPos_clicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(SCREENSHOT_PATH(selfId)) );}
void UiHome::on_btn_allFile_clicked() {  ui->stackedWidget_file->setCurrentIndex(PAGE_FILE_ALL); }
void UiHome::on_btn_friendFile_clicked() {  ui->stackedWidget_file->setCurrentIndex(PAGE_FILE_FRIEND); }
void UiHome::on_btn_groupFile_clicked() { ui->stackedWidget_file->setCurrentIndex(PAGE_FILE_GROUP); }

 //清除所有缓存数据
void UiHome::on_btn_exitLogin_clicked()
{
    updateTimer->stop();
    //退出登录
    QTimer::singleShot(500, this, [=](){
        clearAllData(); // 数据全部清除完再去发信号
        emit userExit();
    });
}

void UiHome::on_btn_autoLogin_clicked()
{
    if(ui->btn_autoLogin->isChecked())
        file->updateAutoLogin(true);
    else
        file->updateAutoLogin(false);
}

void UiHome::on_btn_remainPwd_clicked()
{
    if(ui->btn_remainPwd->isChecked())
        file->updateRemainPwd(true);
    else
        file->updateRemainPwd(false);
}

void UiHome::on_btn_newNotifyVoice_clicked()
{
    if(ui->btn_newNotifyVoice->isChecked())
        file->updateNewNotifyVoice(true);
    else
        file->updateNewNotifyVoice(false);
}

void UiHome::on_btn_groupNotifyVoice_clicked()
{
    if(ui->btn_groupNotifyVoice->isChecked())
        file->updateGroupNotifyVoice(true);
    else
        file->updateGroupNotifyVoice(false);
}

void UiHome::on_btn_selectNewPath_clicked()
{
//    ui->plain_filePath->setPlainText( file->getFilePath() );
}

void UiHome::on_btn_autoUpdate_clicked()
{
    if(ui->btn_autoUpdate->isChecked())
        file->updateAutoUpdate(true);
    else
        file->updateAutoUpdate(false);
}

void UiHome::on_btn_surePasswordModify_clicked()
{
    QString newPassowrd = ui->line_newPassword->text();
    QString sureNewPassowrd = ui->line_sureNewPassword->text();
    if(newPassowrd.isEmpty() || sureNewPassowrd.isEmpty()){
        ui->lab_passwordModifyTip->setText("密码填写不完整，无法修改！");
        return;
    }
    if(newPassowrd != sureNewPassowrd){
        ui->lab_passwordModifyTip->setText("两次密码不相同，无法修改！");
        return;
    }
    QString tip = "密码格式正确。\n请确定是否修改密码?";
    QMessageBox::StandardButton ret = QMessageBox::information(this,"修改密码",tip,QMessageBox::Yes | QMessageBox::No);
    if(QMessageBox::No == ret || QMessageBox::Close == ret){
        clearModifyPassword();
        return;
    }
    QJsonObject obj;
    obj.insert("password", sureNewPassowrd);
    emit resourceRequest(704,0,0,obj);
}

void UiHome::on_btn_resetPassword_clicked()
{
    QString tip = QString("账户：%1\n确定要重置该账户密码吗?").arg(QString::number(selfId));
    QMessageBox::StandardButton ret = QMessageBox::information(this,"重置密码",tip,QMessageBox::Yes | QMessageBox::No);
    if(QMessageBox::No == ret || QMessageBox::Close == ret){
        return;
    }
    emit resourceRequest(705,0,0,"");
}

// 提交建议
void UiHome::on_btn_submitSuggest_clicked()
{
    QString text = ui->textEdit_suggest->toPlainText();
    if(text.isEmpty() || !networkNormal)
        return;

    QJsonObject obj;
    obj.insert("recommend",text);
    emit resourceRequest(77,0,0,obj);
    QString tip = QString("最近提交时间：%1").arg(QDateTime::currentDateTime().toString("MM月dd日 hh:mm:ss"));
    ui->lab_suggestTip->setText(tip);
    ui->textEdit_suggest->clear();
}


