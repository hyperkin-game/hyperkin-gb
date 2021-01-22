#include "funtiontablewidget.h"
#include <QDebug>
#include "constant.h"

#ifdef DEVICE_EVB
int table_item_height = 100;
#else
int table_item_height = 50;
#endif

Funtiontablewidget:: Funtiontablewidget(QWidget *parent) : QTableWidget(parent)
  , m_previousColorRow(-1)
  , m_defaultBkColor(QColor(32, 38, 51))
{
    setStyleSheet("QTableView{background:transparent;}"
                  "QTableView::item:selected{color:white;background:rgb(32, 85, 130);}");
    setAttribute(Qt::WA_TranslucentBackground, true);

    setColumnCount(3);
    setRowCount(4);
    setShowGrid(false);
    verticalHeader()->setVisible(false);
    setMouseTracking(true);
    setFrameShape(QFrame::NoFrame);
    setEditTriggers(QTableWidget::NoEditTriggers);
    setSelectionBehavior(QTableWidget::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);

    connect(this, SIGNAL(cellEntered(int,int)), this,SLOT(listCellEntered(int,int)));
    connect(this, SIGNAL(cellClicked(int,int)), SLOT(listCellClicked(int,int)));
    connect(mainWindow, SIGNAL(retranslateUi()), this, SLOT(retranslateUi()));

#ifdef DEVICE_EVB
    setIconSize(QSize(50, 50));
#else
    setIconSize(QSize(25, 25));
#endif

}

void Funtiontablewidget::retranslateUi(){
    for (int i = 0; i < rowCount(); ++i)
        reflushItemName(i);
}

void Funtiontablewidget::reflushItemName(int i){
    QTableWidgetItem *item = NULL;
    item = new QTableWidgetItem();
    item->setTextColor(QColor(255, 255, 255));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    switch (i) {
    case 0:
        item->setText(tr("Wifi"));
        break;
    case 1:
        item->setText(tr("Hotspot"));
        break;
    case 2:
        item->setText(tr("BlueTooth"));
        break;
    case 3:
        item->setText(tr("Brightness"));
        break;
    case 4:
        item->setText(tr("Volumn"));
        break;
    case 5:
        item->setText(tr("Updater"));
        break;
    case 6:
        item->setText(tr("Language"));
        break;
    default:
        break;
    }

    setItem(i, 2, item);
}

void Funtiontablewidget::addFunctionItems(QStringList &normalicon, QStringList &selectedicon, QStringList &)
{
    m_normalicon = normalicon;
    m_selectedicon = selectedicon;

    for (int i = 0; i < rowCount(); ++i) {
        QTableWidgetItem *item = NULL;
        setItem(i, 0, item = new QTableWidgetItem());
        item = new QTableWidgetItem(QIcon(QPixmap(m_normalicon[i])), QString());
        item->setTextAlignment(Qt::AlignCenter);
        setItem(i, 1, item);
        reflushItemName(i);
        setRowHeight(i, table_item_height);
    }
}

void Funtiontablewidget::listCellEntered(int row, int column)
{
    QTableWidgetItem *it = item(m_previousColorRow, 0);
    if (it != NULL)
        setRowColor(m_previousColorRow, m_defaultBkColor);

    it = item(row, column);
    if (it != NULL)
        setRowColor(row, QColor(45, 53, 66));

    m_previousColorRow = row;
}

void Funtiontablewidget::listCellClicked(int row, int column)
{
    Q_UNUSED(column);
    //    item(row, 1)->setTextColor(QColor(0,0,0));
    changeIcon(row);
    emit currentIndexChanged(row);
}

void Funtiontablewidget::changeIcon(int currentrow)
{
    for (int row = 0; row < rowCount(); row++) {
        QTableWidgetItem *it = item(row, 1);
        if (row == currentrow)
            it->setIcon(QIcon(QPixmap(m_selectedicon[row])));
        else
            it->setIcon(QIcon(QPixmap(m_normalicon[row])));
    }
}

void Funtiontablewidget::setRowColor(int row, const QColor &color) const
{
    for (int col = 0; col < columnCount(); col++) {
        QTableWidgetItem *it = item(row, col);
        it->setBackgroundColor(color);
    }
}

void Funtiontablewidget::leaveEvent(QEvent *event)
{
    QTableWidget::leaveEvent(event);
    listCellEntered(-1, -1);
}

void Funtiontablewidget::resizeEvent(QResizeEvent*)
{
#ifdef DEVICE_EVB
    QHeaderView *headerview = horizontalHeader();
    headerview->setVisible(false);
    headerview->resizeSection(0, 30);
    headerview->resizeSection(1, 60);
    headerview->resizeSection(2, width() - 90);
#else
    QHeaderView *headerview = horizontalHeader();
    headerview->setVisible(false);
    headerview->resizeSection(0, 25);
    headerview->resizeSection(1, 45);
    headerview->resizeSection(2, width() - 70);
#endif
}
