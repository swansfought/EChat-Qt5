#include "headpicture.h"
#include "ui_headpicture.h"

HeadPicture::HeadPicture(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HeadPicture)
{
    ui->setupUi(this);

    // 设置窗口阴影
    QGraphicsDropShadowEffect *shadow;
    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);               // 设置阴影距离
    shadow->setColor(QColor(55, 55, 55));  // 设置阴影颜色
    shadow->setBlurRadius(5);              // 设置阴影圆角
    this->setGraphicsEffect(shadow);

    connect(ui->btn_1,&QPushButton::clicked,this,[=]{ emit pictureIndex(1); });
    connect(ui->btn_2,&QPushButton::clicked,this,[=]{ emit pictureIndex(2); });
    connect(ui->btn_3,&QPushButton::clicked,this,[=]{ emit pictureIndex(3); });
    connect(ui->btn_4,&QPushButton::clicked,this,[=]{ emit pictureIndex(4); });
    connect(ui->btn_5,&QPushButton::clicked,this,[=]{ emit pictureIndex(5); });
    connect(ui->btn_6,&QPushButton::clicked,this,[=]{ emit pictureIndex(6); });
    connect(ui->btn_7,&QPushButton::clicked,this,[=]{ emit pictureIndex(7); });
    connect(ui->btn_8,&QPushButton::clicked,this,[=]{ emit pictureIndex(8); });
    connect(ui->btn_9,&QPushButton::clicked,this,[=]{ emit pictureIndex(9); });
    connect(ui->btn_10,&QPushButton::clicked,this,[=]{ emit pictureIndex(10); });
    connect(ui->btn_11,&QPushButton::clicked,this,[=]{ emit pictureIndex(11); });
    connect(ui->btn_12,&QPushButton::clicked,this,[=]{ emit pictureIndex(12); });
    connect(ui->btn_13,&QPushButton::clicked,this,[=]{ emit pictureIndex(13); });
    connect(ui->btn_14,&QPushButton::clicked,this,[=]{ emit pictureIndex(14); });
    connect(ui->btn_15,&QPushButton::clicked,this,[=]{ emit pictureIndex(15); });
    connect(ui->btn_16,&QPushButton::clicked,this,[=]{ emit pictureIndex(16); });
    connect(ui->btn_17,&QPushButton::clicked,this,[=]{ emit pictureIndex(17); });
    connect(ui->btn_18,&QPushButton::clicked,this,[=]{ emit pictureIndex(18); });
    connect(ui->btn_19,&QPushButton::clicked,this,[=]{ emit pictureIndex(19); });
    connect(ui->btn_20,&QPushButton::clicked,this,[=]{ emit pictureIndex(20); });
    connect(ui->btn_21,&QPushButton::clicked,this,[=]{ emit pictureIndex(21); });
}

HeadPicture::~HeadPicture()
{
    delete ui;
}

void HeadPicture::setPictureType(const int &type)
{
    if(type == 0 || type == 1)
        ui->stackedWidget->setCurrentIndex(type);
}

void HeadPicture::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(245, 246, 247));  // 245, 246, 247
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(QRect(1, 1, this->width() - 2, this->height() - 2), 6, 6);
    QWidget::paintEvent(event);
}
