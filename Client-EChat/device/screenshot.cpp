#include "screenshot.h"

ScreenShot::ScreenShot(QWidget *parent)
    : QMainWindow(parent), m_isMousePressed(false), m_startPos(1,1), m_endPos(1,1)
{
//    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    // 设置窗口属性
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    m_screenPicture = new QPixmap();
    m_screenMenu = new QMenu(this);
    m_screenMenu->addAction("完成", this,[=]{
        // 获取截屏内容
        QRect rect(getCapturedRect(m_startPos, m_endPos));
        QPixmap pixmap = m_screenPicture->copy(rect);
        emit screenImage(pixmap);

        this->hide();
        clearScreenCaptureInfo();
    });
    m_screenMenu->addAction("保存", this,[=]{
        QRect rect(getCapturedRect(m_startPos, m_endPos));
        QString fileName = SCREENSHOT_PATH(selfId) + "/EC截图" + CURRENT_DATETIME + ".png";
        qDebug()<<fileName;
        m_screenPicture->copy(rect).save( fileName , "png");
        this->hide();
        clearScreenCaptureInfo();
    });
    m_screenMenu->addAction("保存全屏", this,[=]{
        QString fileName = SCREENSHOT_PATH(selfId) + "/EC截图" + CURRENT_DATETIME + ".png";
        m_screenPicture->save( fileName , "png");
        this->hide();
        clearScreenCaptureInfo();
    });
}

ScreenShot::~ScreenShot()
{
    if(m_screenPicture != nullptr){
        delete m_screenPicture;
    }
}

void ScreenShot::showEvent(QShowEvent *)
{
    QSize desktopSize = QApplication::desktop()->size();
    QScreen *pscreen = QApplication::primaryScreen();
    *m_screenPicture = pscreen->grabWindow(QApplication::desktop()->winId(), 0, 0, desktopSize.width(), desktopSize.height());

    QPixmap pix(desktopSize.width(), desktopSize.height());
    pix.fill((QColor(0, 0, 0, 150)));
    backgroundPicture = new QPixmap(*m_screenPicture);
    QPainter painter(backgroundPicture);
    painter.drawPixmap(0, 0, pix);
}

QRect ScreenShot::getCapturedRect(QPoint startpos, QPoint endpos)
{
    if(startpos == endpos){
        return QRect();
    }
    QPoint tmpTopLeftPos = startpos, tmpBottomRightPos = endpos;
    if(endpos.x() < startpos.x()){
        tmpBottomRightPos.setX(startpos.x());
        tmpTopLeftPos.setX(endpos.x());
    }
    if(endpos.y() < startpos.y()){
        tmpBottomRightPos.setY(startpos.y());
        tmpTopLeftPos.setY(endpos.y());
    }
    return QRect(tmpTopLeftPos, tmpBottomRightPos);
}

void ScreenShot::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawPixmap(0, 0, *backgroundPicture);

    QRect rect(getCapturedRect(m_startPos, m_endPos));
    if (rect.isValid()) {
        painter.drawPixmap(rect.x(), rect.y(), m_screenPicture->copy(rect));
    }
    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.drawText(rect.x(), rect.y() - 8, tr("%1 x %2 px  |  (Enter快捷完成截图，鼠标右击更多操作！)")
                     .arg(rect.width())
                     .arg(rect.height()));
}

void ScreenShot::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isMousePressed){
        QPoint tmpPos = event->pos();
        m_endPos = tmpPos;
        this->update();
    }
}

void ScreenShot::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        m_isMousePressed = true;
        m_startPos = event->pos();
    }
}

void ScreenShot::mouseReleaseEvent(QMouseEvent *) { m_isMousePressed = false; }

void ScreenShot::contextMenuEvent(QContextMenuEvent *)
{
    this->setCursor(Qt::ArrowCursor);
    m_screenMenu->exec(cursor().pos());
}

void ScreenShot::clearScreenCaptureInfo() { m_startPos = m_endPos; }

void ScreenShot::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape){
        this->hide();
        clearScreenCaptureInfo();
    }else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
        // 获取截屏内容
        QRect rect(getCapturedRect(m_startPos, m_endPos));
        QPixmap pixmap = m_screenPicture->copy(rect);
        emit screenImage(pixmap);

        this->hide();
        clearScreenCaptureInfo();
    }
}
