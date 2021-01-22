#ifndef ABSTRACTWHEELWIDGET_H
#define ABSTRACTWHEELWIDGET_H

#include "basewidget.h"

class AbstractWheelWidget : public BaseWidget
{
    Q_OBJECT
public:
    AbstractWheelWidget(QWidget *parent = 0);
    virtual ~AbstractWheelWidget();

    inline  int currentIndex() const;
    void setCurrentIndex(int index);

    virtual void paintItem(QPainter *painter, int index, QRect &rect) = 0;
    virtual void paintItemMask(QPainter *painter) = 0;

    virtual int itemHeight() const = 0;
    virtual int itemCount() const = 0;

public slots:
    void scrollTo(int index);

signals:
    void stopped(int index);
    void changeTo(int index);

protected:
    virtual void paintEvent(QPaintEvent*);
    bool event(QEvent*);

    int m_currentItem;
    int m_itemOffset; // 0-itemHeight()
    bool m_isScrolled;
    bool m_doSignal;

    QRect m_currentRollrect;
    QFont m_currentMaskFont;
    float m_maskLength;
    QString m_realCurrentText;
};

#endif // ABSTRACTWHEELWIDGET_H
