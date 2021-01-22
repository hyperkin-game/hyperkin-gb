#include "musiclisttable.h"
#include "constant.h"

#include <QHeaderView>

#ifdef DEVICE_EVB
int item_height = 60;
int move_distance_next_step = 300;
#else
int item_height = 35;
int move_distance_next_step = 100;
#endif

#define COLUME_ICON   1
#define COLUME_NAME   2
#define COLUME_SUFFIX 3

MusicListTable::MusicListTable(QWidget *parent) : BaseTableWidget(parent, move_distance_next_step)
  , playingItemIndex(-1)
{    
    init();
    initConnection();
}

void MusicListTable::init()
{
    insertColumn(0);
    insertColumn(1);
    insertColumn(2);
    insertColumn(3);
    insertColumn(4);

    verticalHeader()->setDefaultSectionSize(item_height);
    setContentsMargins(0, 0, 0, 0);
}

void MusicListTable::initConnection()
{
}

void MusicListTable::setRowTextColor(int row, const QColor &color) const
{
    for (int col = 1; col < columnCount() - 1; col++) {
        QTableWidgetItem *it = item(row, col);
        it->setTextColor(color);
    }
}

void MusicListTable::insertIntoTable(const QString &name, const QString &suffix)
{
    int rowcount= rowCount();
    insertRow(rowcount);

    setItem(rowcount, COLUME_ICON, new QTableWidgetItem(QIcon(QPixmap(":/image/music/btn_song_tip.png")), ""));
    setItem(rowcount, COLUME_NAME, new QTableWidgetItem(name));
    setItem(rowcount, COLUME_SUFFIX, new QTableWidgetItem(suffix));

    item(rowcount, COLUME_NAME)->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    item(rowcount, COLUME_SUFFIX)->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
}

void MusicListTable::playingItemChanged(int index)
{
    if (playingItemIndex != -1)
        item(playingItemIndex, COLUME_SUFFIX)->setText(playingItemSuffix);

    if (index != -1) {
        playingItemIndex = index;
        playingItemSuffix = item(index, COLUME_SUFFIX)->text();
        setCurrentCell(index, 0);
        item(index, COLUME_SUFFIX)->setText("★★");
    }
}

void MusicListTable::removeTableItem(int row)
{
    this->removeRow(row);

    if (row < playingItemIndex)
        playingItemIndex--;
    else if (row == playingItemIndex)
        playingItemIndex = -1;
}

void MusicListTable::setOriginState()
{
    setCurrentCell(-1, -1);
    if (this->rowCount() > playingItemIndex && playingItemIndex != -1)
        item(playingItemIndex, COLUME_SUFFIX)->setText(playingItemSuffix);

    playingItemIndex = -1;
}

void MusicListTable::clearTable()
{
    for (int i = rowCount(); i > 0; i--)
        removeRow(0);
}

void MusicListTable::resizeEvent(QResizeEvent *event)
{
#ifdef DEVICE_EVB
    horizontalHeader()->resizeSection(0, 20);
    horizontalHeader()->resizeSection(1, 50);
    horizontalHeader()->resizeSection(2, this->width() - 210);
    horizontalHeader()->resizeSection(3, 120);
    horizontalHeader()->resizeSection(4, 20);
#else
    horizontalHeader()->resizeSection(0, 20);
    horizontalHeader()->resizeSection(1, 20);
    horizontalHeader()->resizeSection(2, this->width() - 120);
    horizontalHeader()->resizeSection(3, 60);
    horizontalHeader()->resizeSection(4, 20);
#endif
    QTableWidget::resizeEvent(event);
}






