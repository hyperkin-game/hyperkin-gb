#include "locallisttable.h"
#include "constant.h"

#include <QHeaderView>

#ifdef DEVICE_EVB
int video_item_height = 60;
int move_distance_next_step = 300;
#else
int video_item_height = 35;
int move_distance_next_step = 100;
#endif

#define COLUME_NAME   0
#define COLUME_SUFFIX 1

LocalListTable::LocalListTable(QWidget *parent) : BaseTableWidget(parent, move_distance_next_step)
  , playingItemIndex(-1)
{
    init();
}

void LocalListTable::init()
{
    insertColumn(0);
    insertColumn(1);

    verticalHeader()->setDefaultSectionSize(video_item_height);
}


void LocalListTable::insertIntoTable(const QString &name, const QString &suffix)
{
    int count = rowCount();
    insertRow(count);

    setItem(count, COLUME_NAME, new QTableWidgetItem(name));
    setItem(count, COLUME_SUFFIX, new QTableWidgetItem(suffix));

    item(count, COLUME_SUFFIX)->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
}

void LocalListTable::playingItemChanged(int index)
{
    if (playingItemIndex != -1)
        item(playingItemIndex, COLUME_SUFFIX)->setText(playingItemSuffix);

    if (index != -1) {
        playingItemIndex = index;
        playingItemSuffix = item(index, COLUME_SUFFIX)->text();
        setCurrentCell(index, 0);
        item(index, COLUME_SUFFIX)->setText(tr("Playing"));
    }
}

void LocalListTable::removeTableItem(int row)
{
    this->removeRow(row);

    if (row < playingItemIndex)
        playingItemIndex--;
    else if (row == playingItemIndex)
        playingItemIndex = -1;
}

void LocalListTable::setOriginState()
{
    setCurrentCell(-1, -1);
    if (this->rowCount() > playingItemIndex && playingItemIndex != -1)
        item(playingItemIndex, COLUME_SUFFIX)->setText(playingItemSuffix);

    playingItemIndex = -1;
}

void LocalListTable::clearTable()
{
    for (int i = rowCount(); i > 0; i--)
        removeRow(0);
}

void LocalListTable::resizeEvent(QResizeEvent *event)
{
#ifdef DEVICE_EVB
    horizontalHeader()->resizeSection(0, width() - 130);
    horizontalHeader()->resizeSection(1, 130);
#else
    horizontalHeader()->resizeSection(0, width() - 70);
    horizontalHeader()->resizeSection(1, 70);
#endif

    QTableWidget::resizeEvent(event);
}
