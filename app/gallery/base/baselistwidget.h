#ifndef BASELISTWIDGET_H
#define BASELISTWIDGET_H

#include <QListWidget>
#include <QMutex>
#include <QMouseEvent>

class BaseListWidget : public QListWidget
{
public:
    BaseListWidget(QWidget *parent = 0);
    void init();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QMutex mutex;
    int scrollBarMaximum;
    QPoint m_pressPoint;
};

#endif // BASELISTWIDGET_H
