#ifndef FUNTIONTABLEWIDGET_H
#define FUNTIONTABLEWIDGET_H

#include <QTableWidget>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>

class Funtiontablewidget : public QTableWidget
{
    Q_OBJECT
public:
    Funtiontablewidget(QWidget *parent = 0);

private:
    int m_previousColorRow;
    QColor m_defaultBkColor;
    QStringList m_normalicon;
    QStringList m_selectedicon;

protected:
    void resizeEvent(QResizeEvent*);
    void reflushItemName(int i);

public:
    void addFunctionItems(QStringList &normalicon, QStringList &selectedicon, QStringList &name);
    void changeIcon(int currentrow);

protected:
    void setRowColor(int row, const QColor &color) const;
    void leaveEvent(QEvent *event);

signals:
    void currentIndexChanged(int index);

public slots:
    void listCellEntered(int row, int column);
    void listCellClicked(int row, int column);
    void retranslateUi();
};

#endif // FUNTIONTABLEWIDGET_H
