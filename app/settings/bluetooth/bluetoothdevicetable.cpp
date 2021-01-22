#include "bluetoothdevicetable.h"
#include "retranslatemanager.h"
#include <QHeaderView>

#ifdef DEVICE_EVB
int move_distance_next_step = 300;
int bluetooth_item_height = 65;
#else
int move_distance_next_step = 100;
int bluetooth_item_height = 35;
#endif

#define COLUME_ICON 1
#define COLUME_NAME 2
#define COLUME_ADDRESS 3
#define COLUME_STATE 4

BluetoothStateItem::BluetoothStateItem(const QString &text) : QTableWidgetItem(text)
{
}

bool BluetoothStateItem::operator <(const QTableWidgetItem &other) const
{
    BtState state = BtState(this->whatsThis().toInt());
    BtState otherState = BtState(other.whatsThis().toInt());

    if (state == BT_STATE_CONNECTED) {
        return false;
    } else if(state == BT_STATE_PAIRED) {
        if (otherState == BT_STATE_CONNECTED)
            return true;
        else
            return false;
    }
    return QTableWidgetItem::operator <(other);
}

BluetoothDeviceTable::BluetoothDeviceTable(QWidget *parent) : BaseTableWidget(parent, move_distance_next_step)
{
    init();
}

void BluetoothDeviceTable::init()
{
    insertColumn(0);
    insertColumn(1);
    insertColumn(2);
    insertColumn(3);
    insertColumn(4);

    verticalHeader()->setDefaultSectionSize(bluetooth_item_height);
}

void BluetoothDeviceTable::insertIntoTable(const QString &name, const QString &address, BtState state)
{
    int rowcount= rowCount();
    insertRow(rowcount);

    setItem(rowcount, COLUME_ICON, new QTableWidgetItem(QIcon(QPixmap(":/image/setting/bluetooth_item_icon.png")),""));
    setItem(rowcount, COLUME_NAME, new QTableWidgetItem(name));
    setItem(rowcount, COLUME_ADDRESS, new QTableWidgetItem(address));

    setItemState(rowcount, state);

    item(rowcount, COLUME_NAME)->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    item(rowcount, COLUME_ADDRESS)->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    item(rowcount, COLUME_STATE)->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    sortTable();
}

void BluetoothDeviceTable::sortTable()
{
    sortItems(COLUME_STATE, Qt::DescendingOrder);
}

void BluetoothDeviceTable::setItemState(int itemRow, BtState state)
{
    switch (state) {
    case BT_STATE_CONNECTED:
        setItem(itemRow, COLUME_STATE, new BluetoothStateItem(QString(str_bluetooth_item_connected)));
        break;
    case BT_STATE_PAIRING:
        setItem(itemRow, COLUME_STATE, new BluetoothStateItem(QString(str_bluetooth_item_pairing)));
        break;
    case BT_STATE_PAIRED:
        setItem(itemRow, COLUME_STATE, new BluetoothStateItem(QString(str_bluetooth_item_paired)));
        break;
    default:
        setItem(itemRow, COLUME_STATE, new BluetoothStateItem(QString(" ")));
        break;
    }

    item(itemRow, COLUME_STATE)->setWhatsThis(QString::number(state));
}

void BluetoothDeviceTable::retranslateTable()
{
    BtState state;
    for (int i = 0; i < rowCount(); i++) {
        state = BtState(item(i, COLUME_STATE)->whatsThis().toInt());
        setItemState(i, state);
    }
}

void BluetoothDeviceTable::clearTable()
{
    for (int i = rowCount(); i > 0; i--)
        removeRow(0);
}

QString BluetoothDeviceTable::getItemName(int row)
{
    return item(row, COLUME_NAME)->text();
}

QString BluetoothDeviceTable::getItemAddress(int row)
{
    return item(row, COLUME_ADDRESS)->text();
}

void BluetoothDeviceTable::resizeEvent(QResizeEvent *event)
{
#ifdef DEVICE_EVB
    horizontalHeader()->resizeSection(0, 20);
    horizontalHeader()->resizeSection(1, 50);
    horizontalHeader()->resizeSection(2, this->width() - 520);
    horizontalHeader()->resizeSection(3, 300);
    horizontalHeader()->resizeSection(4, 150);
#else
    horizontalHeader()->resizeSection(0, 20);
    horizontalHeader()->resizeSection(1, 20);
    horizontalHeader()->resizeSection(2, this->width() - 380);
    horizontalHeader()->resizeSection(3, 220);
    horizontalHeader()->resizeSection(4, 120);
#endif
    QTableWidget::resizeEvent(event);
}
