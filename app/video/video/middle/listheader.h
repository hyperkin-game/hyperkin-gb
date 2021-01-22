#ifndef LISTHEADER_H
#define LISTHEADER_H

#include <QLabel>

#include "basewidget.h"

class FuntionButton : public BaseWidget
{
    Q_OBJECT
public:
    FuntionButton(QString title, QWidget *parent);

    void setFousedStyle();
    void removeFouseStyle();

private:
    QLabel *m_title;
    QFrame *m_bottomLine;

    bool m_isCurItem;
    void initWidget();

protected:
    void mousePressEvent(QMouseEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);

signals:
    void buttonClick();
};

class ListHeader : public BaseWidget
{
    Q_OBJECT
public:
    ListHeader(QWidget *parent = 0);

private:
    FuntionButton *m_button1;
//    FuntionButton *m_button2;

    void initWidget();

public slots:
    void slot_onButtonLocalClick();
    void slot_onButtonNetClick();

signals:
    void buttonLocalClick();
    void buttonNetClick();
};

#endif // LISTHEADER_H
