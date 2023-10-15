#ifndef ITEMCHATOBJECT_H
#define ITEMCHATOBJECT_H

#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QWidget>
#include <QToolTip>
#include <QEvent>
#include <QCursor>
#include <QSoundEffect>
#include <QSound>

#include "global.h"
#include "device/file.h"

namespace Ui {
class ItemChatObject;
}

class ItemChatObject : public QWidget
{
    Q_OBJECT

public:
    explicit ItemChatObject(QWidget *parent = nullptr);
    ~ItemChatObject();

    void setPicture(const int &type,int picture = 0);
    void setNickname(const QString &nickname);
    void setRecentChatTime(const QString &dateTime);
    void setRecentChatMsg(const QString &msg);
    void setRecentUnread(const int &counts);
    void clearRecentUnread();
    void addRecentUnread();
    void setOnlineState(const bool &state);
    void setOnlineStateVisible(const bool &state);
    void setBackground(const bool &state);

    bool getOnlineState();

signals:
    void clicked();
    void removeCliked();
    void topCliked();

protected:
    //    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
//        bool eventFilter(QObject *watched, QEvent *event) override;
    //    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
//    bool event(QEvent *event) override;

private:
    QMenu   *menu;
    bool     keepBackground;

private:
    Ui::ItemChatObject *ui;
};

#endif  // ITEMCHATOBJECT_H
