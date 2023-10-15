#include "item/ItemChatObject.h"

#include "ui_itemchatobject.h"

ItemChatObject::ItemChatObject(QWidget *parent)
    : QWidget(parent), menu(nullptr), keepBackground(false), ui(new Ui::ItemChatObject)
{
    ui->setupUi(this);

    menu = new QMenu(this);
//    menu->addAction("置顶", [=] { emit topCliked(); });
    menu->addAction("移除", [=] { emit removeCliked(); });
}

ItemChatObject::~ItemChatObject(){  delete ui; }

void ItemChatObject::setPicture(const int &type, int picture)
{
//    qDebug()<<picture;
    QString _picture;
    // 群聊
    if(picture <= 0){
        if(type)
            ui->lab_picture->setPixmap(QPixmap(":/img/head/group.png")); //默认头像
        else
            ui->lab_picture->setPixmap(QPixmap(":/img/head/person.png")); //默认头像
    }else{
        QPixmap pixmap;
        _picture = QString(":/img/head/%1.png").arg(QString::number(picture));
        pixmap.load(_picture);
        ui->lab_picture->setPixmap(pixmap);
    }
}

// 2019级计算机...
void ItemChatObject::setNickname(const QString &nickname)
{
    //
    ui->lab_nickname->setText(nickname);
}

// 20:28
void ItemChatObject::setRecentChatTime(const QString &dateTime)
{
    QStringList list = dateTime.split(" ");
    QString time = list.at(1).left(5);
    ui->lab_recentChatTime->setText(time);
    ui->lab_recentChatTime->setToolTip(QString("最近聊天:\n%1 %2").arg(list.at(0).right(5),time));
}

// 晚风来的正巧正巧晚...
void ItemChatObject::setRecentChatMsg(const QString &msg)
{
    QString content = msg;
    content = content.remove("\n");
    ui->lab_recentChatMsg->setText("[最近]" + content);
//    ui->lab_recentChatMsg->setToolTip(msg);
}

// 设置未读消息数量
void ItemChatObject::setRecentUnread(const int &counts)
{
    if (counts <= 0)
        return;

    if (counts <= 99)
        ui->lab_recentUnread->setText(QString("%1+").arg(QString::number(counts)));
    else
        ui->lab_recentUnread->setText("99+");
}

void ItemChatObject::clearRecentUnread() { ui->lab_recentUnread->clear(); }

void ItemChatObject::addRecentUnread()
{
    ++msgCounts;
    QString text = ui->lab_recentUnread->text();
    int unread = text.leftRef(text.length()- 1).toInt() + 1;
    ui->lab_recentUnread->setText(QString("%1+").arg(QString::number( unread )));
}

void ItemChatObject::setOnlineState(const bool &state)
{
    if (state){
        ui->lab_onlineState->setStyleSheet(ONLINE_STATE);
        ui->lab_onlineState->setToolTip("线上");
    }else{
        ui->lab_onlineState->setStyleSheet(OFFLINE_STATE);
        ui->lab_onlineState->setToolTip("已下");
    }
}

void ItemChatObject::setOnlineStateVisible(const bool &state) { ui->lab_onlineState->setVisible(state); }

void ItemChatObject::setBackground(const bool &state)
{
    if (state) {
        ui->widget_background->setStyleSheet(SELECT_BACKGROUND);
        keepBackground = state;
    } else {
        keepBackground = state;
        ui->widget_background->setStyleSheet(LEAVE_BACKGROUND);
    }
}

bool ItemChatObject::getOnlineState()
{
    bool state = false;
    if ("线上" == ui->lab_onlineState->toolTip())
        state = true;
    return state;
}

void ItemChatObject::contextMenuEvent(QContextMenuEvent *event)
{
    menu->exec(QCursor::pos());  // 弹出右键菜单
    event->accept();
}

void ItemChatObject::enterEvent(QEvent *event)
{
    if (!keepBackground)
        ui->widget_background->setStyleSheet(ENTER_BACKGROUND);
}

void ItemChatObject::leaveEvent(QEvent *)
{
    if (!keepBackground)
        ui->widget_background->setStyleSheet(LEAVE_BACKGROUND);
}

void ItemChatObject::mouseReleaseEvent(QMouseEvent *event)
{
    emit clicked();
    QWidget::mouseReleaseEvent(event);
}

//void ItemChatObject::mouseMoveEvent(QMouseEvent *event)
//{
//    Q_UNUSED(event);
//    return;
//}

