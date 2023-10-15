#ifndef DESKTOP_H
#define DESKTOP_H

#include <QApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QScreen>

class Desktop
{
public:
    static Desktop *getInstance() { return new Desktop; }

    QRect pAvailableRect;  // 获取可用桌面大小
    QRect pScreenRect;     // 获取主屏幕分辨率

    //    int              screenCounts;  // 获取屏幕数量
    //    QList<QScreen *> screenList;    // 多屏操作

private:
    QScreen *pScreen;  // 主屏幕

private:
    static Desktop *instance;
    Desktop();
    Desktop(const Desktop &) = delete;
    Desktop &operator=(const Desktop &) = delete;
    ~Desktop();


};

#endif  // DESKTOP_H
