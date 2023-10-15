#ifndef HEADPICTURE_H
#define HEADPICTURE_H

#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QPainter>

namespace Ui {
class HeadPicture;
}

class HeadPicture : public QWidget
{
    Q_OBJECT

public:
    explicit HeadPicture(QWidget *parent = nullptr);
    ~HeadPicture();

    void setPictureType(const int &type); //0好友  1群聊

signals:
    void pictureIndex(const int index);

protected:
    void paintEvent(QPaintEvent *event) override;
//    void mousePressEvent(QMouseEvent *event) override;
//    void mouseReleaseEvent(QMouseEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
//    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::HeadPicture *ui;
};

#endif // HEADPICTURE_H
