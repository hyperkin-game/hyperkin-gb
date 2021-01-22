#include "abstractwheelwidget.h"

#include <QScroller>
#include <QScrollPrepareEvent>
#include <QPainter>

// define a huge number to prevent scroll to the end.
#define WHEEL_SCROLL_OFFSET 50000
#define SCROLL_TIME 200

AbstractWheelWidget::AbstractWheelWidget(QWidget *parent) : BaseWidget(parent)
  , m_currentItem(0)
  , m_itemOffset(0)
  , m_isScrolled(false)
  , m_doSignal(false)
{
}

AbstractWheelWidget::~AbstractWheelWidget()
{
}

int AbstractWheelWidget::currentIndex() const
{
    return m_currentItem;
}

void AbstractWheelWidget::setCurrentIndex(int index)
{
    if (index >= 0 && index < itemCount()) {
        m_currentItem = index;
        m_itemOffset = 0;
        update();
    }
}

bool AbstractWheelWidget::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ScrollPrepare: {
        QScrollPrepareEvent *se = static_cast<QScrollPrepareEvent *>(event);
        se->setViewportSize(QSizeF(size()));
        // we claim a huge scrolling area and a huge content position and
        // hope that the user doesn't notice that the scroll area is restricted
        se->setContentPosRange(QRectF(0.0, 0.0, 0.0, WHEEL_SCROLL_OFFSET*2));
        se->setContentPos(QPointF(0.0, WHEEL_SCROLL_OFFSET + m_currentItem * itemHeight() + m_itemOffset));
        se->accept();

        return true;
    }
    case QEvent::Scroll: {
        QScrollEvent *se = static_cast<QScrollEvent *>(event);

        qreal y = se->contentPos().y();
        int iy = y - WHEEL_SCROLL_OFFSET;
        int ih = itemHeight();

        // -- calculate the current item position and offset and redraw the widget
        int ic = itemCount();
        if (ic > 0) {
            m_currentItem = iy / ih;
            m_itemOffset = iy % ih;

            if (m_currentItem >= ic)
                m_currentItem = ic - 1;
        }

        if (m_doSignal) {
            if (se->scrollState() == QScrollEvent::ScrollStarted) {
                if (m_currentItem > 1)
                    this->m_isScrolled = true;
            }
        }

        if (se->scrollState() == QScrollEvent::ScrollFinished) {
            if (m_doSignal) {
                if (m_currentItem > 1)
                    emit changeTo(this->m_currentItem + 1);
                m_maskLength = 0;
                m_realCurrentText = "";
            }
            this->m_isScrolled = false;
            m_doSignal = true;
            m_itemOffset = 0;
        }

        if (m_itemOffset != 0)
            update();

        se->accept();
        return true;
    }
    case QEvent::MouseButtonPress:
        setFocus();
        return true;
    default:
        return QWidget::event(event);
    }
    return true;
}

void AbstractWheelWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    int w = width();
    int h = height();
    int iH = itemHeight();
    int iC = itemCount();

    if (iC > 0) {
        // just to print the lyric on the height of the widget.
        for (int i = -h/2/iH; i <= h/2/iH; i++) {
            int itemNum = m_currentItem + i;
            if (itemNum >= 0 && itemNum < iC) {
                int len = h / 2 / iH;
                // parabola attenuation ,the value is betweent 220 to 255
                int t = abs(i);
                t = 255 - t * t * 220 / len / len;

                if (t < 0)
                    t = 0;

                painter.setPen(QColor(0, 0, 0, t));
                QRect rect(0, h/2 + i*iH - m_itemOffset, w, iH);
                paintItem(&painter, itemNum, rect);
            }
        }
    }
    //    paintItemMask(&painter);
}

/*!
    Rotates the wheel widget to a given index.
    You can also give an index greater than itemCount or less than zero in which
    case the wheel widget will scroll in the given direction and end up with
    (index % itemCount)
*/
void AbstractWheelWidget::scrollTo(int index)
{
    // users do not use mouse to scroll the lyric content
    this->m_doSignal = false;
    QScroller *scroller = QScroller::scroller(this);
    scroller->scrollTo(QPointF(0, WHEEL_SCROLL_OFFSET + index * itemHeight()), 0);
}
