#ifndef BASETABLEWIDGET_H
#define BASETABLEWIDGET_H

#include <QTableWidget>
#include <QStyledItemDelegate>
#include <QPen>
#include <QObject>
#include <QMutex>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

class QLineDelegate : public QStyledItemDelegate
{
public:
    QLineDelegate(QTableView* tableView);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QPen pen;
    QTableView* view;
};

class BaseTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    BaseTableWidget(QWidget *parent = 0, int moveDistanceNextStep = 100);
    void init();
private:
    // timer to distinguish longPressed event and click event
    QTimer *m_timer;
    int pressedRow;

    int m_moveDistanceNextStep;
    QMutex mutex;
    int scrollBarMaximum;
    QPoint m_pressPoint;
    bool m_longPressedOn;

private slots:
    void onTimerTimeout();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void longPressedEvent(int);
};

#endif // BASETABLEWIDGET_H
