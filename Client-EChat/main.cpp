#include <QApplication>
#include <QSharedMemory>

#include "ui/uilogin.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);//设置最后窗口退出时候退出程序

    //创建共享内存块
    static QSharedMemory *EChat = new QSharedMemory("EChat");
    if (!EChat->create(1)){//创建大小1b的内存
        qApp->quit(); //创建失败，说明已经有一个程序运行，退出当前程序
        return -1;
    }

    QFont font;
    font.setFamily("微软雅黑");
    a.setFont(font);

    UiLogin w;
    w.show();
    w.checkAutoLogin(); //检测自动登录

    return a.exec();
}
