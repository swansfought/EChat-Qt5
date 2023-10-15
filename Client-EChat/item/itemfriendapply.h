#ifndef ITEMFRIENDAPPLY_H
#define ITEMFRIENDAPPLY_H

#include <QWidget>
#include <QEvent>
#include <QToolTip>
#include <QEnterEvent>

namespace Ui {
class ItemFriendApply;
}

class ItemFriendApply : public QWidget
{
    Q_OBJECT

public:
    explicit ItemFriendApply(QWidget *parent = nullptr);
    ~ItemFriendApply();

    void setPicture(const int &type, int picture=0);
    void setNickname(const QString &nickname,const int &id,const int &type);
    void setId(const int &id,const int &type);
    void setPs(const QString &ps);

    int getApplicantId();
    int getGroupId();
    int getType();

    void setAgree(const bool& state);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void agree();
    void refuse();

private slots:
    void on_btn_agree_clicked();
    void on_btn_refuse_clicked();

private:
    Ui::ItemFriendApply *ui;
};

#endif  // ITEMFRIENDAPPLY_H
