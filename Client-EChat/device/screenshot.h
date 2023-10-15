#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QDateTime>
#include <QDir>

#include "global.h"

#define CURRENT_DATETIME QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")

class ScreenShot : public QMainWindow
{
    Q_OBJECT
public:
    explicit ScreenShot(QWidget *parent = nullptr);
    ~ScreenShot();

protected:
    void showEvent(QShowEvent *);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:
    void screenImage(QPixmap pixmap);

private:
    QRect getCapturedRect(QPoint startpos, QPoint endpos);
    void clearScreenCaptureInfo();

    bool m_isMousePressed;
    QPixmap *m_screenPicture;
    QPixmap *backgroundPicture;
    QPoint m_startPos, m_endPos;
    QMenu *m_screenMenu;
};

#endif // SCREENSHOT_H
