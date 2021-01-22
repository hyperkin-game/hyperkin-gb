#ifndef WLANLISTTABLE_H
#define WLANLISTTABLE_H

#include "base/basetablewidget.h"
#include "wpamanager.h"

class WifiStateItem : public QTableWidgetItem
{
public:
    WifiStateItem(const QString &text);

protected:
    virtual bool operator<(const QTableWidgetItem &other) const;

};

class WlanListTable : public BaseTableWidget
{
    Q_OBJECT
public:
    WlanListTable(QWidget *parent = 0);
    void insertIntoTable(const QString &ssid, const QString &bssid, int netWorkId,
                         const QString &signal, const QString &flags, WifiState state);

    QString getItemSSID(int itemRow);
    QString getItemBSSID(int itemRow);
    WifiState getItemState(int itemRow);
    int getItemNetworkId(int itemRow);
    void setItemState(int itemRow, int itemState);
    QString getItemSignalString(int itemRow);

    bool hasSSIDName(const QString &ssidName);
    void clearTable();
    void sortTable();
    void retranslateTable();
    void resetConnnectingItem();

private:
    void init();

protected:
    void resizeEvent(QResizeEvent*);
};

#endif // WLANLISTTABLE_H
