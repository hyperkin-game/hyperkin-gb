#include "basetablewidget.h"

#include <QHeaderView>
#include <QScrollBar>

QLineDelegate::QLineDelegate(QTableView *tableView)
{
    int gridHint = tableView->style()->styleHint(QStyle::SH_Table_GridLineColor, new QStyleOptionViewItemV4());
    QColor gridColor = static_cast<QRgb>(gridHint);
    pen = QPen(gridColor, 0, tableView->gridStyle());
    view = tableView;
}

void QLineDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    QPen oldPen = painter->pen();
    painter->setPen(pen);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->setPen(oldPen);
}

BaseTableWidget::BaseTableWidget(QWidget *parent, int moveDistanceNextStep) : QTableWidget(parent)
  , m_moveDistanceNextStep(moveDistanceNextStep)
{
    init();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
}

void BaseTableWidget::init()
{
    setMouseTracking(true);
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::NoFocus);

    setShowGrid(false);
    setItemDelegate(new QLineDelegate(this));
    setEditTriggers(QTableWidget::NoEditTriggers);
    setSelectionBehavior (QAbstractItemView::SelectRows);
    setSelectionMode (QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    verticalScrollBar()->setStyleSheet("QScrollBar{background:transparent; width: 8px;margin: 0px 0px 0px 0px;}"
                                       "QScrollBar::handle{background:rgb(217,217,217);border-radius:0px;}"
                                       "QScrollBar::handle:hover{background: rgb(191,191,191);}"
                                       "QScrollBar::add-line:vertical{border:1px rgb(230,230,230);height: 0px;}"
                                       "QScrollBar::sub-line:vertical{border:1px rgb(230,230,230);height: 0px;}"
                                       "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background:transparent;}");

    setStyleSheet("QTableWidget{background:rgb(255,255,255)}"
                  "QTableWidget{color:rgb(0,0,0);}"
                  "QTableWidget::item:selected{background:rgb(60,120,190);}"
                  );
}

void BaseTableWidget::onTimerTimeout()
{
    m_timer->stop();
    m_longPressedOn = true;
    emit longPressedEvent(pressedRow);
}

void BaseTableWidget::mousePressEvent(QMouseEvent *event)
{
    QTableWidget::mousePressEvent(event);
    m_pressPoint = event->pos();
    m_longPressedOn = false;

    if (this->itemAt(m_pressPoint) != NULL) {
        pressedRow = this->itemAt(m_pressPoint)->row();
        m_timer->start(1000);
    }
}

void BaseTableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressPoint = QPoint(0, 0);
    m_timer->stop();
    setCurrentCell(-1, -1);

    if (!m_longPressedOn)
        QTableWidget::mouseReleaseEvent(event);
}

void BaseTableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!mutex.tryLock())
        return;

    int currentScrollBarValue;
    int nextScrollBarValue;
    int moveDistance;

    if (m_pressPoint.y() == 0) {
        m_pressPoint = event->pos();
        goto _failed;
    }

    moveDistance = event->pos().y() - m_pressPoint.y();
    if (abs(moveDistance) > m_moveDistanceNextStep) {
        scrollBarMaximum = verticalScrollBar()->maximum();
        currentScrollBarValue = verticalScrollBar()->value();
        if (moveDistance < 0) {
            if (currentScrollBarValue < scrollBarMaximum)
                nextScrollBarValue = currentScrollBarValue + 1;
            else
                goto _failed;
        } else {
            if (currentScrollBarValue > 0)
                nextScrollBarValue = currentScrollBarValue - 1;
            else
                goto _failed;
        }
        verticalScrollBar()->setValue(nextScrollBarValue);
        m_pressPoint = event->pos();
    }

_failed:
    mutex.unlock();
}
