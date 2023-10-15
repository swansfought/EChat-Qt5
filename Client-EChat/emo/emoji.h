#ifndef EMOJI_H
#define EMOJI_H

#include <QWidget>
#include <QPushButton>
#include <QDebug>

#define MAX_ROW 5
#define MAX_COLUMN 12
#define MAX_EMO 60


namespace Ui {
class Emoji;
}

class Emoji : public QWidget
{
    Q_OBJECT

public:
    explicit Emoji(QWidget *parent = nullptr);
    ~Emoji();

    void setMax();
    void setMin();

signals:
    void emo(QString text);

private:
    void gridInsertWidget(QString text,int row,int column);


    Ui::Emoji *ui;
};

#endif // EMOJI_H
