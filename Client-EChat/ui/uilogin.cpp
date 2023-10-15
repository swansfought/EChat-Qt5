#include "ui/uilogin.h"

#include "ui_uilogin.h"

UiLogin::UiLogin(QWidget *parent) :
    QMainWindow(parent),
    isDrag(false),
    loginTipNum(0),
    userLoginFlag(false),
    verifyResult(0),
    pwdIsOpen(false),
    ui(new Ui::UiLogin)
{
    ui->setupUi(this);

    // 设置窗口属性和窗口大小
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setMinimumHeight(372);
    this->setMaximumHeight(372);
    this->setWindowTitle("EChat");

    // 设置窗口阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);               // 设置阴影距离
    shadow->setColor(QColor(55, 55, 55));  // 设置阴影颜色
    shadow->setBlurRadius(5);              // 设置阴影圆角
    ui->widget_loginBackground->setGraphicsEffect(shadow);

    // 加载样式表
    QFile styleFile(":/qss/login.qss");
    if(styleFile.open(QFile::ReadOnly)){
        QTextStream stream(&styleFile);
        QString     styleSheet = styleFile.readAll();
        this->setStyleSheet(styleSheet);
        styleFile.close();
    }
    ui->widget_top->installEventFilter(this);
    ui->widget_title->installEventFilter(this);
    ui->lab_verify->installEventFilter(this);

    // 文件操作
    file = File::getInstance();
    file->checkConfigFile(); //检查配置文件

    // 注册弹出动画
    QPropertyAnimation *animation = new QPropertyAnimation(ui->widget_register,"pos");
    animation->setDuration(500);
    animation->setEndValue(QPoint(2,130));
    connect(ui->btn_goRegister,&QPushButton::clicked,this,[=]{
        clearRegisterInfo(); //清除上次遗留的注册信息
        setVertifyCode();
        if(380 == ui->widget_register->pos().y()){
            animation->setStartValue(QPoint(2,380));
            animation->setEndValue(QPoint(2,130));
            animation->start();
        }
    });
    connect(ui->btn_closeRegister,&QPushButton::clicked,this,[=]{
        if(130 == ui->widget_register->pos().y()){
            animation->setStartValue(QPoint(2,130));
            animation->setEndValue(QPoint(2,380));
            animation->start();
        }
    });

    // 设置输入框密码隐藏
    ui->line_loginPwd->setEchoMode(QLineEdit::Password);
    ui->line_registerPwd->setEchoMode(QLineEdit::Password);
    connect(ui->line_loginPwd,&QLineEdit::returnPressed,this,[=]{ on_btn_login_clicked();});

    // 选项框的处理
    connect(ui->cbox_autoLogin, &QCheckBox::clicked, this, [=]() {
        if (ui->cbox_autoLogin->isChecked()) {
            ui->cbox_remainderPwd->setChecked(true);
            file->updateAutoLogin(true); //按键选中更新配置文件
            file->updateRemainPwd(true);
        }else{
            file->updateAutoLogin(false);
        }
    });
    connect(ui->cbox_remainderPwd, &QCheckBox::clicked, this, [=]() {
        if (ui->cbox_remainderPwd->isChecked()){
            file->updateRemainPwd(true);
        }else{
            ui->cbox_autoLogin->setChecked(false);
            file->updateRemainPwd(false);
        }
    });
    connect(ui->com_loginAccount,&QComboBox::currentTextChanged,this,[=]{ ui->line_loginPwd->clear(); });
    connect(ui->com_loginAccount,&QComboBox::editTextChanged,this,[=]{ ui->line_loginPwd->clear(); });

    //重连接加载图
    movie = new QMovie(":/img/skypeLoader.gif",QByteArray(),this); //lineLoading.gif threeLoading.gif
    ui->lab_connecting->setMovie(movie);
    ui->widget_connecting->setVisible(false);

    uiHome = new UiHome; // 主界面初始化

    // 创建客户端子线程
//    qDebug() << "ui tid: " << QThread::currentThreadId();
    Client  *client = new Client;
    QThread *clientThread = new QThread;
    client->moveToThread(clientThread);
    // 连接服务器
    connect(clientThread, &QThread::started, client, &Client::on_connectServer);
    connect(client, &Client::connectState, this, &UiLogin::on_connectState);
    connect(client, &Client::connectState, uiHome, &UiHome::on_connectState);
    //    connect(this, &UiLogin::connectServer, client, &Client::on_connectServer);

    // 发送消息
    connect(this, &UiLogin::newUserLogin, client, &Client::on_newUserLogin);
    connect(this, &UiLogin::loginRequest, client, &Client::on_loginRequest);
    connect(this, &UiLogin::registerRequest, client, &Client::on_registerRequest);
    connect(this, &UiLogin::loginSuccess, uiHome, &UiHome::on_loginSuccess);
    connect(uiHome, &UiHome::userExit, this, &UiLogin::on_userExit);
    connect(uiHome, &UiHome::resourceRequest, client, &Client::on_resourceRequest);
    connect(uiHome, &UiHome::sendChatMsg, client, &Client::on_sendChatMsg);
    connect(uiHome, &UiHome::sendChatFile, client, &Client::on_sendChatFile);

    // 回应消息
    connect(client, &Client::loginReply, this, &UiLogin::on_loginReply);
    connect(client, &Client::registerReply, this, &UiLogin::on_registerReply);
    connect(client, &Client::fileSendFinish, uiHome, &UiHome::on_fileSendFinish);
    connect(client, &Client::resourceReply, uiHome, &UiHome::on_resourceReply);

    // 资源释放
    connect(this, &UiLogin::uiLoginClose, clientThread, [=] {
        //        ui->btn_close->setStyleSheet("#btn_close{background-color:rgb(240, 91, 87);}");
        clientThread->quit();
        clientThread->wait();
    });
    connect(clientThread, &QThread::finished, client, [=] {
        client->deleteLater();
        clientThread->deleteLater();
        emit allowUiClose();
        qDebug() << "clientThread资源释放完成！";
    });
    connect(this, &UiLogin::allowUiClose, this, [=]{
        auto db = DataBase::getInstance();
        db->disconnectDB();
        qApp->exit(); //结束程序
    });
    clientThread->start();

    // -------其他设置-----------
    addNickNameMap(); // 初始化随机昵称
    createystemTrayIcon(); //创建系统托盘
    setInputRegExp();

    //登录提示定时器
    timer = new QTimer(this);
    timer->setInterval(500);
    connect(timer,&QTimer::timeout,this,&UiLogin::on_loginTip);
}

UiLogin::~UiLogin()
{
//    if (nullptr != uiHome) {
//        delete uiHome;
//        uiHome = nullptr;
//    }
    qDebug() << "UiLogin资源释放完成！";
    delete ui;
}

void UiLogin::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(255, 255, 255));
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(QRect(2, 2, this->width() - 4, this->height() - 4), 5, 5);
    QMainWindow::paintEvent(event);
}

void UiLogin::mousePressEvent(QMouseEvent *event)
{
    offset = event->globalPos() - this->pos();
    offy = event->globalPos().y() - this->pos().y();
    QMainWindow::mousePressEvent(event);
}

void UiLogin::mouseReleaseEvent(QMouseEvent *event)
{
    offset = QPoint();
    QMainWindow::mouseReleaseEvent(event);
}

void UiLogin::mouseMoveEvent(QMouseEvent *event)
{
    if (!isDrag || QPoint() == offset)
        return;
    move(event->globalPos() - offset);
    QMainWindow::mouseMoveEvent(event);
}

bool UiLogin::eventFilter(QObject *watched, QEvent *event)
{
    //重置验证码
    if(dynamic_cast<QWidget*>(watched) == ui->lab_verify && event->type() == QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); //事件转换
        if( Qt::LeftButton == mouseEvent->button() ){
            setVertifyCode();
        }
    }
    //指定位置拖动
    if(event->type() == QEvent::Enter){
        if(dynamic_cast<QWidget*>(watched) == ui->widget_top || dynamic_cast<QWidget*>(watched) == ui->widget_title)
            this->isDrag = true;
    }
    if(event->type() == QEvent::Leave){
        if(dynamic_cast<QWidget*>(watched) == ui->widget_top || dynamic_cast<QWidget*>(watched) == ui->widget_title)
            this->isDrag = false;
    }
    return QWidget::eventFilter(watched,event);
}

// 连接服务器失败
void UiLogin::on_connectState(bool state)
{
    if (state) {
        ui->widget_connecting->setVisible(false);
        movie->stop();
        ui->lab_loginTip->clear();
        ui->lab_registerTip->clear();
    } else {
        movie->start();
        ui->widget_connecting->setVisible(true);
        ui->lab_loginTip->setText("网络已断开，请检查网络配置！");
        ui->lab_registerTip->setText("网络已断开，请检查网络配置！");
    }
}

// 用户验证
void UiLogin::on_loginReply(QByteArray json)
{
    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "on_verifyUserReply json parse error: " << parseError.errorString();
        return;
    }
    QJsonObject rootObj = doc.object();
    bool        state = rootObj["state"].toBool();

    if (!state) {
        ui->lab_loginTip->setText("账户或者密码错误，请重新核实!");
        //重置提示
        timer->stop();
        loginTipNum = 0;
        ui->btn_login->setText("登录");
        return;
    }
    // 用户已经登录情况
//    if(rootObj["loginState"].toBool()) {
//        //重置提示
//        timer->stop();
//        loginTipNum = 0;
//        ui->btn_login->setText("登录");
//        QString tip = QString("[账号]%1\n此账号已登录！").arg(QString::number(rootObj["id"].toInt()));
//        QMessageBox::information(this,"登陆提示",tip,QMessageBox::Ok);
//        return;
//    }

    // 用户未登录情况
    emit newUserLogin(); //先去判断文件描述符情况！
    ui->lab_loginTip->clear();
    file->checkDataFolder(); //如果是其他用户则需要检测该用户存放数据的文件夹是否存在
    emit loginSuccess(); // 通知uiHome界面加载数据
    userLoginFlag = true;
}

// 注册用户
void UiLogin::on_registerReply(QByteArray json)
{
    if(!networkNormal)
        return;
    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "on_addUserReply json parse error: " << parseError.errorString();
        return;
    }
    QJsonObject rootObj = doc.object();
    bool        state = rootObj["state"].toBool();
    if (!state) {
        ui->lab_registerTip->setText("注册失败，请重新尝试！");
    } else {
        ui->lab_registerTip->clear();
        int         newId = rootObj["newId"].toInt();

        QString tip = QString("新账户：%1\n新昵称：%2\n注册成功，请记住EC号！").arg(QString::number(newId),ui->line_nickName->text());
        QMessageBox::information(this,"账户注册",tip,QMessageBox::Ok);

        // 清除登录信息并且返回登录界面
        ui->line_loginPwd->clear();
//        ui->cbox_autoLogin->setCheckState(Qt::Unchecked);
//        ui->cbox_remainderPwd->setCheckState(Qt::Unchecked);
        ui->com_loginAccount->addItem( QString::number(newId) );
        ui->com_loginAccount->setCurrentText( QString::number(newId) );

        //点击返回按钮，去触发动画效果
        ui->btn_closeRegister->click();
    }
}

void UiLogin::on_loginTip()
{
    static int counts = 0;
    if(0 == loginTipNum){
        ui->btn_login->setText("登录中.");
    }else if(1 == loginTipNum){
        ui->btn_login->setText("登录中..");
    }else if(2 == loginTipNum){
        ui->btn_login->setText("登录中...");
    }
    if(2 == loginTipNum)
        loginTipNum = 0;
    else
        loginTipNum++;
    counts++;

    // 4s登录提示完成，显示home界面
    if(LOGIN_TIP_COUNT == counts){
        timer->stop();
        uiHome->show();
        this->hide();
//        uiHome->activateWindow(); //设置为活动窗口
        ui->btn_login->setText("登录");

        //重置提示
        counts = 0;
        loginTipNum = 0;

        // 把新id添加到登录账户中   ??有问题的！
//        QString id = QString::number(selfId);
//        for (int i = 0; i < ui->com_loginAccount->count(); ++i) {
//            if(id != ui->com_loginAccount->itemText(i))
//                ui->com_loginAccount->insertItem(0,id);
//        }
    }
}

//用户退出登录
void UiLogin::on_userExit()
{
//    ui->cbox_autoLogin->setChecked(false);
//    file->updateAutoLogin(false);

    // 没有记住密码情况
    if(!ui->cbox_remainderPwd->isChecked())
        ui->line_loginPwd->clear();
    // 用户修改密码退出情况
    if(modifyPassword)
        ui->line_loginPwd->clear();

    this->show();
    uiHome->hide();
}

// 关闭窗口
void UiLogin::on_btn_close_clicked() { this->close(); }

void UiLogin::on_btn_forgetPassword_clicked()
{
//    QString text = QInputDialog::getText(this,"请输入EC号","请正确输入EC号")


//    QString tip = QString("账户：%1\n确定要重置该账户密码吗?").arg(QString::number(selfId));
//    QMessageBox::StandardButton ret = QMessageBox::information(this,"重置密码",tip,QMessageBox::Yes | QMessageBox::No);
//    if(QMessageBox::No == ret || QMessageBox::Close == ret){
//        return;
//    }
//    emit resourceRequest(705,0,0,"");
}

void UiLogin::setInputRegExp()
{
    //账户/验证结果：纯数字
    QRegExp numReg(NUM_REG);
    ui->com_loginAccount->setValidator(new QRegExpValidator(numReg,this));
    ui->line_verifyResult->setValidator(new QRegExpValidator(numReg,this));

    //密码：英文大小写+数字
    QRegExp pwdReg(PWD_REG);
    ui->line_loginPwd->setValidator(new QRegExpValidator(pwdReg,this));
    ui->line_registerPwd->setValidator(new QRegExpValidator(pwdReg,this));
}

void UiLogin::setVertifyCode()
{
    int first_num = QRandomGenerator::global()->bounded(0, 99);
    int second_num = QRandomGenerator::global()->bounded(0, 66);
    ui->lab_verify->setText(QString("%1 + %2").arg(QString::number(first_num), QString::number(second_num)));
    verifyResult = first_num + second_num;
}

void UiLogin::clearRegisterInfo()
{
    // 注册信息
    if(networkNormal)
        ui->lab_registerTip->clear();
    ui->line_nickName->clear();
    ui->line_registerPwd->clear();
    ui->lab_verify->clear();
    ui->line_verifyResult->clear();
    ui->btn_pwdIsOpen->setStyleSheet("#btn_pwdIsOpen{image:url(:/img/pwdClose.png);}");
    ui->line_registerPwd->setEchoMode(QLineEdit::Password);
    pwdIsOpen = false;
}

void UiLogin::createystemTrayIcon()
{
    QSystemTrayIcon* systemTrayIcon = new QSystemTrayIcon(this);
    QMenu* menu = new QMenu(this);
    QCommonStyle style;
    menu->addAction("打开界面");
    menu->addAction(QIcon(style.standardPixmap(QStyle::SP_DockWidgetCloseButton)),"退出");
    systemTrayIcon->setContextMenu(menu);
    systemTrayIcon->setIcon(QIcon(":/img/EChat.ico"));
    systemTrayIcon->setToolTip("EChat");
    systemTrayIcon->show();
    connect(systemTrayIcon,&QSystemTrayIcon::activated,this,[=](QSystemTrayIcon::ActivationReason reason){
        if(reason == QSystemTrayIcon::Trigger) {  //针对不同窗口点击事件处理
            if(userLoginFlag){
                uiHome->show(); //已经登录了，是登陆界面
                uiHome->activateWindow();
            }else{
                this->show();
            }
        }
//        if(reason == QSystemTrayIcon::DoubleClick)
    });
    connect(menu,&QMenu::triggered,this,[=](QAction *action){
        if("打开界面" == action->text()){
            if(userLoginFlag){
                uiHome->show(); //已经登录了，是登陆界面
                uiHome->activateWindow();
            }else{
                this->show();
            }
        }else if("退出" == action->text()){
            uiHome->deleteLater(); //释放home界面
            emit uiLoginClose();
        }
    });
}

// 函数在构造函数中
void UiLogin::checkAutoLogin()
{
    bool autoLogin = file->getAutoLogin();
    bool remainPwd = file->getRemainPwd();
    //登陆界面选项勾选
    if(autoLogin)
        ui->cbox_autoLogin->setChecked( autoLogin );
    if(remainPwd)
        ui->cbox_remainderPwd->setChecked( remainPwd );

    int userId = file->getEchatId();
    QString userPassword = file->getPassword();
    bool isOk = true;
    //自动登录初始化设置
    if(0 != userId){
        ui->com_loginAccount->addItem(QString::number( userId ) );
        if(!userPassword.isEmpty() && remainPwd){
            ui->line_loginPwd->setText( userPassword );
            ui->line_loginPwd->setFocus();
        }
    }else
        isOk = false;

    // 开始自动登录
    if(autoLogin && isOk){
        // 延迟一下
        QTimer::singleShot(600, this, [=](){ on_btn_login_clicked(); });
    }else{
        if(ui->line_loginPwd->text().isEmpty())
            ui->com_loginAccount->setFocus();//设置账户输入为当前焦点，前提密码有输入
    }
}

// 登录账户
void UiLogin::on_btn_login_clicked()
{
    if(!networkNormal || timer->isActive())
        return;

    if (ui->com_loginAccount->currentText().isEmpty() || ui->line_loginPwd->text().isEmpty()) {
        ui->lab_loginTip->setText("账户或密码不能为空!");
        return;
    }
    int     id = ui->com_loginAccount->currentText().toInt();
    QString pwd = ui->line_loginPwd->text();

    emit loginRequest(id, pwd);
    timer->start(); //开启登录提示

    //更新配置文件中的用户id和密码
    file->updateEchatId(id);
    file->updatePassword(pwd);
}

// 注册账户
void UiLogin::on_btn_sureRegister_clicked()
{
    emit newUserLogin(); //先去判断文件描述符情况！
    // 输入数据核验
    if (ui->line_nickName->text().isEmpty() || ui->line_registerPwd->text().isEmpty()) {
        if(networkNormal)
            ui->lab_registerTip->setText("昵称或密码为空，请补充填写!");
        return;
    }
    if (ui->line_verifyResult->text().isEmpty() || verifyResult != ui->line_verifyResult->text().toInt()) {
        if(networkNormal){
            ui->lab_registerTip->setText("验证结果错误，请重新填写!");
            ui->line_verifyResult->clear();
        }
        return;
    }
    QString name = ui->line_nickName->text();
    QString pwd = ui->line_registerPwd->text();

    emit registerRequest(name, pwd);
}

// 产生随机昵称，显示随机昵称
void UiLogin::on_btn_reflushNickName_clicked()
{
    int  index = QRandomGenerator::global()->bounded(0, 21);
    auto it = nickNameMap.find(index);
    if (nickNameMap.end() == it)
        return;
    ui->line_nickName->setText(it.value());
}

// 显示密码
void UiLogin::on_btn_pwdIsOpen_clicked()
{
    if (!pwdIsOpen) {
        ui->btn_pwdIsOpen->setStyleSheet("#btn_pwdIsOpen{image:url(:/img/pwdOpen.png);}");
        ui->line_registerPwd->setEchoMode(QLineEdit::Normal);
    } else {
        ui->btn_pwdIsOpen->setStyleSheet("#btn_pwdIsOpen{image:url(:/img/pwdClose.png);}");
        ui->line_registerPwd->setEchoMode(QLineEdit::Password);
    }
    pwdIsOpen = !pwdIsOpen;
}

// 添加部分随机昵称
void UiLogin::addNickNameMap()
{
    nickNameMap.insert(0, "捞月的渔民");
    nickNameMap.insert(1, "荆棘原野");
    nickNameMap.insert(2, "暗哑于秋");
    nickNameMap.insert(3, "半夜汽笛");
    nickNameMap.insert(4, "黑夜的海");
    nickNameMap.insert(5, "樱笋年光");
    nickNameMap.insert(6, "孤赏");
    nickNameMap.insert(7, "无心");
    nickNameMap.insert(8, "月染杉");
    nickNameMap.insert(9, "自在安然");
    nickNameMap.insert(10, "青舟弄酒");
    nickNameMap.insert(11, "集市漫街巷");
    nickNameMap.insert(12, "晚春里");
    nickNameMap.insert(13, "风起半山");
    nickNameMap.insert(14, "北子栀");
    nickNameMap.insert(15, "槐序廿柒");
    nickNameMap.insert(16, "小草泠");
    nickNameMap.insert(17, "迷路的信儿");
    nickNameMap.insert(18, "星霜荏苒");
    nickNameMap.insert(19, "长谷深风");
    nickNameMap.insert(20, "落日余晖");
    nickNameMap.insert(21, "橘柚");
}
