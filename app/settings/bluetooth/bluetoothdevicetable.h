#ifndef BLUETOOTHDEVICETABLE_H
#define BLUETOOTHDEVICETABLE_H

#include "base/basetablewidget.h"

enum BtState {
    BT_STATE_NULL = 0,
    BT_STATE_PAIRING,
    BT_STATE_PAIRED,
    BT_STATE_CONNECTED,
};

class BluetoothStateItem : public QTableWidgetItem
{
public:
    BluetoothStateItem(const QString &text);

protected:
    virtual bool operator<(const QTableWidgetItem &other) const;
};

class BluetoothDeviceTable : public BaseTableWidget
{
    Q_OBJECT
public:
    BluetoothDeviceTable(QWidget *parent);

    void insertIntoTable(const QString &name, const QString &address, BtState state);
    void clearTable();
    void sortTable();
    void retranslateTable();

    QString getItemName(int row);
    QString getItemAddress(int row);
    void setItemState(int itemRow, BtState state);

private:
    void init();

protected:
    void resizeEvent(QResizeEvent*);
};

#endif // BLUETOOTHDEVICETABLE_H
