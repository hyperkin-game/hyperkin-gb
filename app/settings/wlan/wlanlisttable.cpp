#include "wlanlisttable.h"

#include <QHeaderView>
#include <QScrollBar>

#include "constant.h"
#include "retranslatemanager.h"

#ifdef DEVICE_EVB
int next_step_move_distance = 100;
int wlan_item_height = 65;
#else
int next_step_move_distance = 30;
int wlan_item_height = 35;
#endif

#define COLUME_SSID 1
#define COLUME_STATE 2
#define COLUME_SIGNAL 3
#define COLUME_LOCK_STATE 4

WifiStateItem::WifiStateItem(const QString &text) : QTableWidgetItem(text)
{
}

bool WifiStateItem::operator <(const QTableWidgetItem &other) const
{
    WifiState state = WifiState(this->whatsThis().toInt());
    WifiState otherState = WifiState(other.whatsThis().toInt());

    if (state == WIFI_STATE_CONNECTED || state == WIFI_STATE_CONNECTING) {
        return false;
    }  else if (state == WIFI_STATE_SAVED || this->text() == WIFI_STATE_AUTH_FAILED) {
        if (otherState == WIFI_STATE_CONNECTED || otherState == WIFI_STATE_CONNECTING)
            return true;
        else
            return false;
    }
    return QTableWidgetItem::operator <(other);
}

WlanListTable::WlanListTable(QWidget *parent) : BaseTableWidget(parent, next_step_move_distance)
{
    init();
}

void WlanListTable::init()
{
    insertColumn(0);
    insertColumn(1);
    insertColumn(2);
    insertColumn(3);
    insertColumn(4);

    this->setIconSize(QSize(27, 27));
    verticalHeader()->setDefaultSectionSize(wlan_item_height);
}

void WlanListTable::insertIntoTable(const QString &ssid, const QString &bssid, int netWorkId,
                                    const QString &signal, const QString &flags, WifiState state)
{
    int rowcount = rowCount();
    insertRow(rowcount);

    setItem(rowcount, COLUME_SSID, new QTableWidgetItem(ssid));
    item(rowcount, COLUME_SSID)->setTextAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    /* carry wifi bssid information in colume "SSID" */
    item(rowcount, COLUME_SSID)->setWhatsThis(bssid);

    setItemState(rowcount, state);
    item(rowcount, COLUME_STATE)->setTextAlignment(Qt::AlignVCenter|Qt::AlignLeft);

    QTableWidgetItem *siganlItem;
    int signalValue = signal.toInt();
    if (signalValue >= -55)
        siganlItem = new QTableWidgetItem(QIcon(QPixmap(":/image/setting/ic_wifi_signal_4_dark.png")), NULL);
    else if(signalValue >= -70)
        siganlItem = new QTableWidgetItem(QIcon(QPixmap(":/image/setting/ic_wifi_signal_3_dark.png")), NULL);
    else if(signalValue >= -85)
        siganlItem = new QTableWidgetItem(QIcon(QPixmap(":/image/setting/ic_wifi_signal_2_dark.png")), NULL);
    else
        siganlItem = new QTableWidgetItem(QIcon(QPixmap(":/image/setting/ic_wifi_signal_1_dark.png")), NULL);

    setItem(rowcount, COLUME_SIGNAL, siganlItem);
    item(rowcount, COLUME_SIGNAL)->setData(Qt::DisplayRole, signalValue);

    /* carry wifi networkId information in colume "SIGNAL" */
    item(rowcount, COLUME_SIGNAL)->setWhatsThis(QString::number(netWorkId));

    QString auth;
    if (flags.indexOf("[WPA2-EAP") >= 0)
        auth = "WPA2_EAP";
    else if (flags.indexOf("[WPA-EAP") >= 0)
        auth = "WPA_EAP";
    else if (flags.indexOf("[WPA2-PSK") >= 0)
        auth = "WPA2_PSK";
    else if (flags.indexOf("[WPA-PSK") >= 0)
        auth = "WPA_PSK";
    else
        auth = "OPEN";

    if (auth.compare("OPEN") == 0)
        setItem(rowcount, COLUME_LOCK_STATE, new QTableWidgetItem(QIcon(QPixmap(":/image/setting/ic_wifi_unlocked.png")), NULL));
    else
        setItem(rowcount, COLUME_LOCK_STATE, new QTableWidgetItem(QIcon(QPixmap(":/image/setting/ic_wifi_locked.png")), NULL));

    sortTable();
}

void WlanListTable::clearTable()
{
    for (int i = rowCount(); i > 0; i--)
        removeRow(0);
}

void WlanListTable::retranslateTable()
{
    WifiState state;
    for (int i = 0; i < rowCount(); i++) {
        state = getItemState(i);
        setItemState(i, state);
    }
}

void WlanListTable::sortTable()
{
    // sort table twice.
    // one for signal value and one for connect state.
    sortByColumn(COLUME_SIGNAL);
    sortItems(COLUME_STATE, Qt::DescendingOrder);
}

QString WlanListTable::getItemSSID(int itemRow)
{
    return this->item(itemRow, COLUME_SSID)->text();
}

QString WlanListTable::getItemBSSID(int itemRow)
{
    return this->item(itemRow, COLUME_SSID)->whatsThis();
}

WifiState WlanListTable::getItemState(int itemRow)
{
    return WifiState(this->item(itemRow, COLUME_STATE)->whatsThis().toInt());
}

void WlanListTable::setItemState(int itemRow, int itemState)
{
    switch (itemState) {
    case WIFI_STATE_SAVED:
        setItem(itemRow, COLUME_STATE, new WifiStateItem(QString(str_wifi_item_saved)));
        break;
    case WIFI_STATE_AUTH_FAILED:
        setItem(itemRow, COLUME_STATE, new WifiStateItem(QString(str_wifi_item_failed)));
        break;
    case WIFI_STATE_CONNECTING:
        setItem(itemRow, COLUME_STATE, new WifiStateItem(QString(str_wifi_item_connecting)));
        break;
    case WIFI_STATE_CONNECTED:
        setItem(itemRow, COLUME_STATE, new WifiStateItem(QString(str_wifi_item_connected)));
        break;
    default:
        setItem(itemRow, COLUME_STATE, new WifiStateItem(QString(" ")));
        break;
    }

    item(itemRow, COLUME_STATE)->setWhatsThis(QString::number(itemState));
}

int WlanListTable::getItemNetworkId(int itemRow)
{
    return this->item(itemRow, COLUME_SIGNAL)->whatsThis().toInt();
}

QString WlanListTable::getItemSignalString(int itemRow)
{
    int signalValue = this->item(itemRow, COLUME_SIGNAL)->data(Qt::DisplayRole).toInt();

    if (signalValue >= -55)
        return QString(str_wifi_signal_excellent);
    else if(signalValue >= -70)
        return QString(str_wifi_signal_good);
    else if(signalValue >= -85)
        return QString(str_wifi_signal_ok);
    else
        return QString(str_wifi_signal_weak);
}

bool WlanListTable::hasSSIDName(const QString &ssidName)
{
    for (int i = 0; i < rowCount(); i++) {
        if (this->item(i, COLUME_SSID)->text() == ssidName)
            return true;
    }
    return false;
}

void WlanListTable::resetConnnectingItem()
{
    for (int i = 0; i < rowCount(); i++) {
        if (this->item(i, COLUME_STATE)->whatsThis().toInt() == WIFI_STATE_CONNECTING) {
            setItemState(i, WIFI_STATE_SAVED);
            return;
        }
    }
}

void WlanListTable::resizeEvent(QResizeEvent *event)
{
#ifdef DEVICE_EVB
    QTableWidget::resizeEvent(event);
    horizontalHeader()->resizeSection(0, 20);
    horizontalHeader()->resizeSection(1, width() - 400);
    horizontalHeader()->resizeSection(2, 300);
    horizontalHeader()->resizeSection(3, 30);
    horizontalHeader()->resizeSection(4, 50);
#else
    QTableWidget::resizeEvent(event);
    horizontalHeader()->resizeSection(0, 20);
    horizontalHeader()->resizeSection(1, width() - 200);
    horizontalHeader()->resizeSection(2, 100);
    horizontalHeader()->resizeSection(3, 30);
    horizontalHeader()->resizeSection(4, 50);
#endif
}
