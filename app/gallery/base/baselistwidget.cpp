#include "baselistwidget.h"

#include <QScrollBar>

#ifdef DEVICE_EVB
int move_distance_step = 300;
#else
int move_distance_step = 100;
#endif

BaseListWidget::BaseListWidget(QWidget *parent) : QListWidget(parent)
{
    init();
}

void BaseListWidget::init()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    verticalScrollBar()->setStyleSheet("QScrollBar{background:transparent; width: 20px;margin: 0px 0px 0px 0px;}"
                                       "QScrollBar::handle{background:rgb(217,217,217);border-radius:0px;}"
                                       "QScrollBar::handle:hover{background: rgb(191,191,191);}"
                                       "QScrollBar::add-line:vertical{border:1px rgb(230,230,230);height: 0px;}"
                                       "QScrollBar::sub-line:vertical{border:1px rgb(230,230,230);height: 0px;}"
                                       "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background:transparent;}");

    /* cancel the border in ListWidget */
    setFrameShape(QListWidget::NoFrame);
    /* set QListWidget be IconMode to show image thumnail and unable to drag */
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setStyleSheet("QListWidget::indicator{subcontrol-position:top;}"
                  "QListWidget{background:transparent}"
                  "QListWidget::item{padding:2px;border:0px solid gray;}"
                  "QListWidget::item:selected:active{padding:2px;background:red}");


}

void BaseListWidget::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);

    m_pressPoint = event->pos();
}

void BaseListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if  (!mutex.tryLock())
        return;

    int currentScrollBarValue;
    int nextScrollBarValue;
    int moveDistance;

    if (m_pressPoint.y() == 0) {
        m_pressPoint = event->pos();
        goto _failed;
    }

    moveDistance = event->pos().y() - m_pressPoint.y();
    if (abs(moveDistance) > move_distance_step) {
        scrollBarMaximum = verticalScrollBar()->maximum();
        currentScrollBarValue = verticalScrollBar()->value();
        if (moveDistance < 0) {
            if (currentScrollBarValue < scrollBarMaximum)
                nextScrollBarValue = currentScrollBarValue + move_distance_step;
            else
                goto _failed;
        }else{
            if (currentScrollBarValue > 0)
                nextScrollBarValue = currentScrollBarValue - move_distance_step;
            else
                goto _failed;
        }
        verticalScrollBar()->setValue(nextScrollBarValue);
        m_pressPoint = event->pos();
    }

_failed:
    mutex.unlock();
}
