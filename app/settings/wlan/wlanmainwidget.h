#ifndef MAINSURFACE_H
#define MAINSURFACE_H

#include "basewidget.h"
#include "wpamanager.h"
#include "base/switchbutton.h"
#include "wlanlisttable.h"

#include <QLabel>

class SwitchHeader : public BaseWidget
{
    Q_OBJECT
public:
    SwitchHeader(QWidget *parent = 0);
    void setChecked(bool checked);

private:
    QLabel *m_switchText;
    SwitchButton *m_switchButton;

signals:
    void sig_checkedChanged(bool);
};

class WlanMainWidget : public BaseWidget
{
    Q_OBJECT
public:
    WlanMainWidget(QWidget *parent = 0);
    ~WlanMainWidget(){}

private:
    WPAManager *m_manager;
    SwitchHeader *m_switchHeader;
    WlanListTable *m_table;

    void initData();
    void initLayout();
    void initConnection();

protected:
    void showEvent(QShowEvent*);

private slots:
    void slot_checkStateChanged(bool);

    void slot_onScanResultComeIn(QList<netWorkItem>);
    void slot_onTableItemClick(int, int);
    void slot_onItemConnecting(QString itemSSID);
    void slot_onItemConnectFailed(QString itemSSID);
    void slot_onItemConnectComplete(QString itemBSSID);
    void slot_onItemDisconnected(QString itemBSSID);
    void retranslateUi();
};

#endif // MAINSURFACE_H
