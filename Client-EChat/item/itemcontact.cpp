#include "item/itemcontact.h"

#include "ui_itemcontact.h"

ItemContact::ItemContact(QWidget *parent) : QWidget(parent), keepBackground(false), ui(new Ui::ItemContact) {
    ui->setupUi(this);
    this->setMinimumSize(QSize(242,50));
    this->setMaximumSize(QSize(242,50));
}

ItemContact::~ItemContact() {  delete ui;}

void ItemContact::setPicture(const int &type,int picture)
{
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

void ItemContact::setNickname(const QString &nickname) { ui->lab_nickname->setText(nickname); }

void ItemContact::setId(const int &id) { ui->lab_id->setText(QString::number(id)); }

void ItemContact::setApplicants(const int &counts)
{
    ui->lab_id->setText(QString("%1+").arg(QString::number(counts)));
}

void ItemContact::removeApplicants()
{
    QString tip = ui->lab_id->text();
    int counts = tip.leftRef(tip.length() - 1).toInt() - 1;
    if(0 == counts)
        ui->lab_id->clear();
    else{
        ui->lab_id->setText(QString("%1+").arg(QString::number(counts)));
    }
}

void ItemContact::setBackground(const bool &state)
{
    if (state) {
        ui->widget_background->setStyleSheet(SELECT_BACKGROUND);
        keepBackground = state;
    } else {
        keepBackground = state;
        ui->widget_background->setStyleSheet(LEAVE_BACKGROUND);
    }
}

void ItemContact::setTipStyle()
{
    QFont font;
    font.setFamily("微软雅黑");
    font.setPointSize(9);
    font.setBold(true);
    ui->lab_id->setFont(font);
    ui->lab_id->setAlignment(Qt::AlignRight);
}

int ItemContact::getContactId() { return ui->lab_id->text().toInt(); }

void ItemContact::enterEvent(QEvent *)
{
    if (!keepBackground)
        ui->widget_background->setStyleSheet(ENTER_BACKGROUND);
}

void ItemContact::leaveEvent(QEvent *)
{
    if (!keepBackground)
        ui->widget_background->setStyleSheet(LEAVE_BACKGROUND);
}

void ItemContact::mouseReleaseEvent(QMouseEvent *event)
{
    emit clicked();
    QWidget::mouseReleaseEvent(event);
}

void ItemContact::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}
