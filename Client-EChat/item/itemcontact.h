#ifndef ITEMCONTACT_H
#define ITEMCONTACT_H

#include <QWidget>

#include "global.h"
#include <QDebug>

namespace Ui {
class ItemContact;
}

class ItemContact : public QWidget
{
    Q_OBJECT

public:
    explicit ItemContact(QWidget *parent = nullptr);
    ~ItemContact();

    void setPicture(const int &type,int picture=0);
    void setNickname(const QString &nickname);
    void setId(const int &id);
    void setApplicants(const int &counts);
    void removeApplicants();
    void setBackground(const bool &state);
    void setTipStyle();

    int getContactId();

signals:
    void clicked();

protected:
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    //    bool eventFilter(QObject *watched, QEvent *event) override;
    //    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    bool keepBackground;

private:
    Ui::ItemContact *ui;
};

#endif  // ITEMCONTACT_H
