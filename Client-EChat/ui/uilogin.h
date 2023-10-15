#ifndef UILOGIN_H
#define UILOGIN_H

#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QMainWindow>
#include <QMap>
#include <QMouseEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QCommonStyle>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QSharedPointer>
#include <QTextStream>
#include <QThread>
#include <QEvent>
#include <QTimer>

#include "network/client.h"
#include "device/file.h"
#include "ui/uihome.h"
#include "global.h"

#define LOGIN_TIP_COUNT 8

namespace Ui {
class UiLogin;
}

class UiLogin : public QMainWindow
{
    Q_OBJECT

public:
    UiLogin(QWidget *parent = nullptr);
    ~UiLogin();
    void checkAutoLogin();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
//    void closeEvent(QCloseEvent *event) override;

signals:
    void connectServer();
    void uiLoginClose();
    void allowUiClose();  // 资源释放
    void newUserLogin();
    void loginSuccess();

    // --客户端请求--
    void loginRequest(const int &userId, const QString &userPwd);
    void registerRequest(const QString &nickname, const QString &newNpwd);

public slots:
    void on_connectState(bool state);

    // --处理服务器回应--
    void on_loginReply(QByteArray json);     // 101：验证用户
    void on_registerReply(QByteArray json);  // 102：注册用户

private slots:
    void on_loginTip();
    void on_userExit();
    void on_btn_reflushNickName_clicked();  // 随机昵称-√
    void on_btn_pwdIsOpen_clicked();        // 显隐密码-√
    void on_btn_login_clicked();            // 登录账户-√
    void on_btn_sureRegister_clicked();     // 注册账户-√
    void on_btn_close_clicked();            // 关闭窗口-√
    void on_btn_forgetPassword_clicked();   //忘记密码

private:
    void setInputRegExp();
    void setVertifyCode();
    void clearRegisterInfo();
    void createystemTrayIcon();
    void addNickNameMap();

private:
    QPoint         offset;        //
    int            offy;          //
    bool           isDrag;
    QMovie         *movie;
    int            loginTipNum;
    QTimer*        timer;
    bool           userLoginFlag; //系统托盘点击显示界面判断标志

    int                verifyResult;  // 验证计算结果
    bool               pwdIsOpen;     // 密码是否可见，默认不可见
    QMap<int, QString> nickNameMap;   // 昵称

    File *file;
    UiHome  *uiHome;  // 主界面ui
    //    QSharedPointer<UiHome> uiHome;

private:
    Ui::UiLogin *ui;
};
#endif  // UILogin_H
