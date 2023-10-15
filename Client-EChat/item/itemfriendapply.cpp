#include "item/itemfriendapply.h"

#include "ui_itemfriendapply.h"

ItemFriendApply::ItemFriendApply(QWidget *parent) : QWidget(parent), ui(new Ui::ItemFriendApply)
{
    ui->setupUi(this);
    ui->lab_nickname->installEventFilter(this);
    ui->lab_ps->installEventFilter(this);
}

ItemFriendApply::~ItemFriendApply() { delete ui; }

void ItemFriendApply::setPicture(const int &type, int picture)
{
    if (picture <= 0){
        if(type)
            ui->lab_picture->setPixmap(QPixmap(":/img/head/group.png")); //默认头像
        else
            ui->lab_picture->setPixmap(QPixmap(":/img/head/person.png")); //默认头像
    }else{
        QString _picture = QString(":/img/head/%1.png").arg(QString::number(picture));
        QPixmap pixmap;
        pixmap.load(_picture);
        ui->lab_picture->setPixmap(pixmap);
    }
}

void ItemFriendApply::setNickname(const QString &nickname,const int &id,const int &type)
{
    if(type){
        ui->lab_nickname->setText(nickname +"[" + QString::number(id) + "]");
        //tooltip
    }
    else{
        ui->lab_nickname->setText(nickname);
    }
}

void ItemFriendApply::setId(const int &id,const int &type)
{
    if(type)
        ui->lab_id->setText("[群]" + QString::number(id));
    else
        ui->lab_id->setText("[友]"+ QString::number(id));
}

void ItemFriendApply::setPs(const QString &ps)
{
    if(ps.isEmpty())
        ui->lab_ps->setText("附:你好呀！");
    else {
        QString _ps = "附:" + ps;;
        ui->lab_ps->setText(_ps);
        if(ps.length() > 7)
            ui->lab_ps->setToolTip(_ps);
    }
}

int ItemFriendApply::getApplicantId()
{
//    int id;
//    if(type){
//        QStringList list = ui->lab_nickname->text().split("[");
//        id = list.at(1).leftRef(7).toInt();
//    }else
//        id = ui->lab_id->text().rightRef(7).toInt();

    return ui->lab_id->text().rightRef(7).toInt();
}

int ItemFriendApply::getGroupId()
{
    return ui->lab_id->text().rightRef(7).toInt();
}

int ItemFriendApply::getType()
{
    QString type = ui->lab_id->text().left(3);
    if("[群]" == type)
        return 1;
    return 0;
}

void ItemFriendApply::setAgree(const bool &state)
{
    ui->btn_agree->setEnabled(false);
    ui->btn_refuse->setEnabled(false);
    if(state){
        ui->btn_agree->setText("已同意");
    }else{
        ui->btn_refuse->setText("已拒绝");
    }
}

bool ItemFriendApply::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::ToolTip){
        if(dynamic_cast<QLabel*>(watched) == ui->lab_id || dynamic_cast<QLabel*>(watched) == ui->lab_nickname)
            QToolTip::showText(QCursor::pos(), toolTip(), this);
//            QToolTip::showText(dynamic_cast<QEnterEvent*>(event)->globalPos(), toolTip(), this); //QEvent::Enter
    }
    return QWidget::eventFilter(watched, event);
}

void ItemFriendApply::on_btn_agree_clicked() { emit agree(); }

void ItemFriendApply::on_btn_refuse_clicked() { emit refuse(); }

