#include "wlanmainwidget.h"
#include "wpaserviceutil.h"
#include "netconnectdialog.h"
#include "netconfigdialog.h"
#include "netconnectedinfodialog.h"
#include "retranslatemanager.h"
#include "constant.h"
#include "qkeyboard.h"

#include <QVBoxLayout>

#ifdef DEVICE_EVB
int wlan_bottom_margin = 500;
#else
int wlan_bottom_margin = 80;
#endif

SwitchHeader::SwitchHeader(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(92, 89, 89);

    QHBoxLayout *layout = new QHBoxLayout;

    m_switchText = new QLabel(this);
    setWidgetFontBold(m_switchText);

    m_switchButton = new SwitchButton(this);
#ifdef DEVICE_EVB
    m_switchButton->setFixedSize(100, 40);
#else
    m_switchButton->setFixedSize(50, 23);
#endif

    layout->addWidget(m_switchText);
    layout->addStretch(0);
    layout->addWidget(m_switchButton);
    setContentsMargins(20, 0, 20, 0);
    setLayout(layout);

    connect(m_switchButton, SIGNAL(checkedChanged(bool)), this, SIGNAL(sig_checkedChanged(bool)));
}

void SwitchHeader::setChecked(bool checked)
{
    m_switchButton->setChecked(checked);
    m_switchText->setText(checked ? QString(str_wifi_open) : QString(str_wifi_close));
}

WlanMainWidget::WlanMainWidget(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(33, 36, 43);

    initData();
    initLayout();
    initConnection();
}

void WlanMainWidget::initData()
{
    creat_supplicant_file();
    m_manager = WPAManager::getInstance(this);

/* TODO: we shoud need set it for fullscreen mode */ 
#ifdef DEVICE_EVB
    QKeyBoard::getInstance()->globalInit(QKeyBoard::Black, 40, 6);
#else
    QKeyBoard::getInstance()->globalInit(QKeyBoard::Black, 45, 16);
#endif
}

void WlanMainWidget::initLayout()
{
    QVBoxLayout *mainlayout = new QVBoxLayout;

    m_switchHeader = new SwitchHeader(this);

    m_table = new WlanListTable(this);

    mainlayout->addSpacing(30);
    mainlayout->addWidget(m_switchHeader);
    mainlayout->addWidget(m_table);
    mainlayout->addSpacing(wlan_bottom_margin);
    mainlayout->setMargin(0);
    mainlayout->setSpacing(0);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch(1);
    layout->addLayout(mainlayout, 8);
    layout->addStretch(1);

    setLayout(layout);
}

void WlanMainWidget::initConnection()
{
    connect(m_manager, SIGNAL(sig_scanResultAvailable(QList<netWorkItem>)), this, SLOT(slot_onScanResultComeIn(QList<netWorkItem>)));
    connect(m_manager, SIGNAL(sig_eventConnecting(QString)), this, SLOT(slot_onItemConnecting(QString)));
    connect(m_manager, SIGNAL(sig_eventConnectFail(QString)), this, SLOT(slot_onItemConnectFailed(QString)));
    connect(m_manager, SIGNAL(sig_eventConnectComplete(QString)), this, SLOT(slot_onItemConnectComplete(QString)));
    connect(m_manager, SIGNAL(sig_eventDisconnected(QString)), this, SLOT(slot_onItemDisconnected(QString)));

    connect(m_switchHeader, SIGNAL(sig_checkedChanged(bool)), this, SLOT(slot_checkStateChanged(bool)));
    connect(m_table, SIGNAL(cellClicked(int,int)), this, SLOT(slot_onTableItemClick(int,int)));

    connect(mainWindow, SIGNAL(retranslateUi()), this, SLOT(retranslateUi()));
}

void WlanMainWidget::retranslateUi()
{
    m_switchHeader->setChecked(is_supplicant_running());
    m_table->retranslateTable();
}

void WlanMainWidget::slot_checkStateChanged(bool checked)
{
    m_switchHeader->setChecked(checked);
    if (checked) {
        m_manager->openCtrlConnection("wlan0");
        m_manager->startWPAConnection();
        wifi_stop_hostapd();
        wifi_start_supplicant();
        QTimer::singleShot(500, m_manager, SLOT(scan()));
    } else {
        m_table->clearTable();
        m_manager->closeWPAConnection();
        wifi_stop_supplicant();
    }
}

void WlanMainWidget::slot_onScanResultComeIn(QList<netWorkItem> list)
{
    qDebug("WLAN: scan result come in: %d", list.size());
    m_table->clearTable();
    QList<netWorkItem> configuredItems = m_manager->getConfiguredNetWork();

    for (int i = 0; i < list.size(); i++) {
        netWorkItem item = list.at(i);

        if (item.ssid == "" || m_table->hasSSIDName(item.ssid))
            continue;

        for (int j = 0; j < configuredItems.size(); j++) {
            if (configuredItems.at(j).ssid == item.ssid) {
                item.state = configuredItems.at(j).state;
                item.networkId = configuredItems.at(j).networkId;
                break;
            }
        }
        m_table->insertIntoTable(item.ssid, item.bssid, item.networkId, item.signal, item.flags, item.state);
    }
}

void WlanMainWidget::slot_onTableItemClick(int row, int)
{
    WifiState itemState = m_table->getItemState(row);
    QString itemSSID = m_table->getItemSSID(row);

    if (itemState == WIFI_STATE_NULL) {
        NetConnectDialog *dialog = new NetConnectDialog(this);
        dialog->setWifiName(itemSSID);

        int result = dialog->exec();
        if (result == NetConnectDialog::RESULT_CONNECT) {
            // supplicant add_network and setlect_network
            m_manager->connectNetwork(itemSSID, dialog->getEditText());
        }
    } else if (itemState == WIFI_STATE_CONNECTED) {
        int result = NetConnectedInfoDialog::showDialog(this, itemSSID, m_table->getItemSignalString(row));
        if (result == NetConnectedInfoDialog::RESULT_CANCEL_SAVE) {
            m_manager->removeNetwork(m_table->getItemNetworkId(row));
            m_table->setItemState(row, WIFI_STATE_NULL);
            m_table->sortTable();
        }
    } else {
        int result = NetConfigDialog::showDialog(this, itemSSID);
        if (result == NetConfigDialog::RESULT_CONNECT) {
            m_manager->selectNetwork(QString::number(m_table->getItemNetworkId(row)));
        } else if (result == NetConfigDialog::RESULT_CANCEL_SAVE) {
            m_manager->removeNetwork(m_table->getItemNetworkId(row));
            m_table->setItemState(row, WIFI_STATE_NULL);
            m_table->sortTable();
        }
    }
}

void WlanMainWidget::slot_onItemConnecting(QString itemSSID)
{
    qDebug("WLAN: item connecting: %s", itemSSID.toLocal8Bit().data());
    for (int i = 0; i < m_table->rowCount(); i++) {
        if (m_table->getItemSSID(i) == itemSSID) {
            m_table->resetConnnectingItem();
            m_table->setItemState(i, WIFI_STATE_CONNECTING);
            m_table->sortTable();
            return;
        }
    }
}

void WlanMainWidget::slot_onItemConnectFailed(QString itemSSID)
{
    qDebug("WLAN: item connect failed with ssid: %s", itemSSID.toLocal8Bit().data());
    for (int i = 0; i < m_table->rowCount(); i++) {
        if (m_table->getItemSSID(i) == itemSSID) {
            m_table->setItemState(i, WIFI_STATE_AUTH_FAILED);
            m_table->sortTable();
            return;
        }
    }
}

void WlanMainWidget::slot_onItemConnectComplete(QString itemBSSID)
{
    qDebug("WLAN: item connect complete with bssid: %s", itemBSSID.toLocal8Bit().data());
    for (int i = 0; i < m_table->rowCount(); i++) {
        if (m_table->getItemBSSID(i) == itemBSSID) {
            get_IP_address();
            m_table->setItemState(i, WIFI_STATE_CONNECTED);
            m_table->sortTable();
            return;
        }
    }
}

void WlanMainWidget::slot_onItemDisconnected(QString itemBSSID)
{
    qDebug("WLAN: item disconnected with bssid: %s", itemBSSID.toLocal8Bit().data());
    for (int i = 0; i < m_table->rowCount(); i++) {
        if (m_table->getItemBSSID(i) == itemBSSID && m_table->getItemState(i) == WIFI_STATE_CONNECTED) {
            m_table->setItemState(i, WIFI_STATE_SAVED);
            m_table->sortTable();
            return;
        }
    }
}

void WlanMainWidget::showEvent(QShowEvent *event)
{
    BaseWidget::showEvent(event);

    bool wifiStationOn = is_supplicant_running();
    m_switchHeader->setChecked(wifiStationOn);

    if (wifiStationOn) {
        m_manager->scan();
    } else {
        m_table->clearTable();
        m_manager->closeWPAConnection();
    }
}
