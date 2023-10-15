#include "emoji.h"
#include "ui_emoji.h"

Emoji::Emoji(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Emoji)
{
    ui->setupUi(this);

//    char32_t  t[] = {0x1F601,0x0};	//加上0x00防止表情后面跟随乱码
//    for(char32_t i = 0x1F601 ; i <=  0x1F64F ; ++i)
//    {
//        t[0] = i;
//        QString::fromUcs4(t);
//    }

    int row = 0;
    int column = 0;
    char32_t  emojiChar[] = {0x1F601,0x0};	//加上0x00防止表情后面跟随乱码
    char32_t emojiUnicode = 0x1F601;

    // 第一页
    for(int i=0; i<MAX_EMO/2; i++) {
        if(column == MAX_COLUMN/2){
            ++row;
            column = 0;
        }
        if(row == MAX_ROW){
            break;
        }
        emojiChar[0] = emojiUnicode++;
        gridInsertWidget(QString::fromUcs4(emojiChar), row, column);
        ++column;
    }

    // 第二页
    row = 0;
    column = 6;
    for(int i=0; i<MAX_EMO/2; i++) {
        if(column == MAX_COLUMN){
            ++row;
            column = 6;
        }
        if(row == MAX_ROW)
            break;
        emojiChar[0] = emojiUnicode++;
        gridInsertWidget(QString::fromUcs4(emojiChar), row, column);
        ++column;
    }
}

Emoji::~Emoji()
{
    delete ui;
}

void Emoji::setMax()
{
    this->setMinimumSize(QSize(342,156));
    this->setMaximumSize(QSize(342,156));
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void Emoji::setMin()
{
    this->setMinimumSize(QSize(203,156));
    this->setMaximumSize(QSize(203,156));
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void Emoji::gridInsertWidget(QString text,int row,int column)
{
    QPushButton *btn = new QPushButton(this);
    QFont font;
    font.setFamily("微软雅黑");
    font.setPointSize(10);
    font.setStyleStrategy(QFont::PreferAntialias);
    btn->setFont(font);
    btn->setText(text);
    btn->setMaximumSize(QSize(28,28));
    btn->setMinimumSize(QSize(28,28));

    ui->gridLayout_emoji->addWidget(btn,row,column);
    connect(btn,&QPushButton::clicked,this,[=]{ emit emo(btn->text()); }); //表情包信号
}


