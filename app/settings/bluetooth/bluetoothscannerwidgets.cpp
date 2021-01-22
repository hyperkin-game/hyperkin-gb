#include "bluetoothscannerwidgets.h"

#include <QHBoxLayout>
#include <QLabel>
#include "constant.h"
#include "base/cmessagebox.h"
#include "pairedinformationdialog.h"
#include "retranslatemanager.h"

#ifdef DEVICE_EVB
int bluetooth_layout_stretch = 15;
int bluetooth_switch_width = 100;
int bluetooth_switch_height = 40;
int bluetooth_refresh_size = 50;
int bottom_margin = 500;
#else
int bluetooth_layout_stretch = 8;
int bluetooth_switch_width = 50;
int bluetooth_switch_height = 23;
int bluetooth_refresh_size = 30;
int bottom_margin = 60;
#endif

BluetoothSwitchHeader::BluetoothSwitchHeader(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(92, 89, 89);

    QHBoxLayout *layout = new QHBoxLayout;

    m_switchText = new QLabel(this);
    setWidgetFontBold(m_switchText);

    m_switchButton = new SwitchButton(this);
    m_switchButton->setFixedSize(bluetooth_switch_width, bluetooth_switch_height);

    layout->addWidget(m_switchText);
    layout->addStretch(0);
    layout->addWidget(m_switchButton);
    setContentsMargins(20, 0, 20, 0);
    setLayout(layout);

    connect(m_switchButton, SIGNAL(checkedChanged(bool)), this, SIGNAL(sig_checkedChanged(bool)));
}

void BluetoothSwitchHeader::setChecked(bool checked)
{
    m_switchButton->setChecked(checked);
}

void BluetoothSwitchHeader::setTitle(const QString &title)
{
    m_switchText->setText(str_bluetooth_title.arg(title));
}

BluetoothScannerWidgets::BluetoothScannerWidgets(QWidget *parent) : BaseWidget(parent)
  , firstEnter(true)
{
    setBackgroundColor(33, 36, 43);

    initData();
    initLayout();
    initConnection();
    retranslateUi();
}

void BluetoothScannerWidgets::initData()
{
    m_localDevice = new QBluetoothLocalDevice;

    /*
     * In case of multiple Bluetooth adapters it is possible to set adapter
     * which will be used. Example code:
     *
     * QBluetoothAddress address("XX:XX:XX:XX:XX:XX");
     * discoveryAgent = new QBluetoothDeviceDiscoveryAgent(address);
     *
     **/
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    m_bluez5Helper = new Bluez5Helper(this);
}

void BluetoothScannerWidgets::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // 1. bluetooth switch.
    m_switchHeader = new BluetoothSwitchHeader(this);

    // 2. refresh layout
    availableTitle = new QLabel(this);
    availableTitle->setText(str_bluetooth_enable_devices);

    m_refreshButton = new RotatableButton(":/image/setting/bluetooth_scan.png", this);
    m_refreshButton->setFixedSize(bluetooth_refresh_size, bluetooth_refresh_size);

    QHBoxLayout *refreshLayout = new QHBoxLayout;
    refreshLayout->addWidget(availableTitle);
    refreshLayout->addStretch(0);
    refreshLayout->addWidget(m_refreshButton);
    refreshLayout->setContentsMargins(25, 5, 25, 5);

    BaseWidget *refreshWidget = new BaseWidget(this);
    refreshWidget->setBackgroundColor(69, 75, 83);
    refreshWidget->setLayout(refreshLayout);

    // 3. bluetooth device list
    m_table = new BluetoothDeviceTable(this);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_switchHeader);
    mainLayout->addWidget(refreshWidget);
    mainLayout->addWidget(m_table);
    mainLayout->addSpacing(bottom_margin);
    mainLayout->setSpacing(0);

    // set layout in middle.
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch(1);
    layout->addLayout(mainLayout, bluetooth_layout_stretch);
    layout->addStretch(1);

    setLayout(layout);
}

void BluetoothScannerWidgets::initConnection()
{
    connect(m_switchHeader, SIGNAL(sig_checkedChanged(bool)),
            this, SLOT(slot_onBluetoothSwitchCheckChanged(bool)));
    connect(m_discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(slot_addDevice(QBluetoothDeviceInfo)));
    connect(m_discoveryAgent, SIGNAL(canceled()), this, SLOT(slot_scanFinished()));
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(slot_scanFinished()));
    connect(m_localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)),
            this, SLOT(slot_hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    connect(m_localDevice, SIGNAL(pairingFinished(QBluetoothAddress,QBluetoothLocalDevice::Pairing)),
            this, SLOT(slot_paringDone(QBluetoothAddress,QBluetoothLocalDevice::Pairing)));
    connect(m_localDevice, SIGNAL(deviceConnected(QBluetoothAddress)),
            this, SLOT(slot_onDeviceConnected(QBluetoothAddress)));
    connect(m_localDevice, SIGNAL(deviceDisconnected(QBluetoothAddress)),
            this, SLOT(slot_onDeviceDisconnected(QBluetoothAddress)));
    slot_hostModeStateChanged(m_localDevice->hostMode());

    connect(m_refreshButton, SIGNAL(clicked(bool)), this, SLOT(slot_reScan()));
    connect(m_table, SIGNAL(cellClicked(int,int)), this, SLOT(slot_onTableItemClicked(int,int)));
    connect(m_table, SIGNAL(longPressedEvent(int)), this, SLOT(slot_onTableItemlongPressed(int)));

    connect(mainWindow, SIGNAL(retranslateUi()), this, SLOT(retranslateUi()));
}

void BluetoothScannerWidgets::retranslateUi()
{
    m_switchHeader->setTitle(m_localDevice->name());
    availableTitle->setText(str_bluetooth_enable_devices);
    m_table->retranslateTable();
}

void BluetoothScannerWidgets::slot_onBluetoothSwitchCheckChanged(bool checked)
{
    if (checked)
        m_localDevice->powerOn();
    else
        m_localDevice->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
}

void BluetoothScannerWidgets::slot_addDevice(const QBluetoothDeviceInfo &info)
{
    QList<QTableWidgetItem*> items = m_table->findItems(info.address().toString(), Qt::MatchContains);
    if (items.empty()) {
        QBluetoothLocalDevice::Pairing pairingStatus = m_localDevice->pairingStatus(info.address());
        if (pairingStatus == QBluetoothLocalDevice::Paired ||
                pairingStatus == QBluetoothLocalDevice::AuthorizedPaired) {
            if (m_bluez5Helper->isDeviceConnected(info.address()))
                m_table->insertIntoTable(info.name(), info.address().toString(), BT_STATE_CONNECTED);
            else
                m_table->insertIntoTable(info.name(), info.address().toString(), BT_STATE_PAIRED);
        } else
            m_table->insertIntoTable(info.name(), info.address().toString(), BT_STATE_NULL);
    }
}

void BluetoothScannerWidgets::slot_onTableItemClicked(int row, int)
{
    int result;
    QString name = m_table->getItemName(row);
    QString address = m_table->getItemAddress(row);
    QBluetoothAddress bluetoothAddress(address);

    QBluetoothLocalDevice::Pairing pairingStatus = m_localDevice->pairingStatus(bluetoothAddress);
    if (pairingStatus == QBluetoothLocalDevice::Paired ||
            pairingStatus == QBluetoothLocalDevice::AuthorizedPaired ) {
        if (!m_bluez5Helper->isDeviceConnected(bluetoothAddress)) {
            result = CMessageBox::showCMessageBox(mainWindow, str_bluetooth_connect_with + name.append(" ?"),
                                                  str_bluetooth_connect, str_bluetooth_cancel);
            if (result == CMessageBox::RESULT_CONFIRM)
                m_bluez5Helper->connectDevice(bluetoothAddress);
        } else {
            result = CMessageBox::showCMessageBox(mainWindow, str_bluetooth_disconnect_with + name.append(" ?"),
                                                  str_bluetooth_disconnect, str_bluetooth_cancel);
            if (result == CMessageBox::RESULT_CONFIRM)
                m_bluez5Helper->disconnectDevice(bluetoothAddress);
        }
    } else {
        result = CMessageBox::showCMessageBox(mainWindow, str_bluetooth_pair_with + name.append(" ?"),
                                              str_bluetooth_pair, str_bluetooth_cancel);
        if (result == CMessageBox::RESULT_CONFIRM) {
            m_localDevice->requestPairing(bluetoothAddress, QBluetoothLocalDevice::Paired);
            m_table->setItemState(row, BT_STATE_PAIRING);
        }
    }
}

void BluetoothScannerWidgets::slot_onTableItemlongPressed(int row)
{
    QString name = m_table->getItemName(row);
    QString address = m_table->getItemAddress(row);
    QBluetoothAddress bluetoothAddress(address);

    if (m_localDevice->pairingStatus(bluetoothAddress) != QBluetoothLocalDevice::Unpaired) {
        int result = PairedInformationDialog::showInformationDialog(this, name, address);
        if (result == PairedInformationDialog::RESULT_CANCELSAVE)
            m_localDevice->requestPairing(bluetoothAddress, QBluetoothLocalDevice::Unpaired);
    }
}

void BluetoothScannerWidgets::slot_scanFinished()
{
    m_refreshButton->stopAnimation();
    qDebug("Bluetooth: scan finished");
}

void BluetoothScannerWidgets::clearDiscoveredDevices()
{
    QList<QBluetoothDeviceInfo> discoveredDevices = m_discoveryAgent->discoveredDevices();
    for (int i = 0; i < discoveredDevices.size(); i++)
        m_bluez5Helper->removeUnpairedDevice(discoveredDevices.at(i).address());
}

void BluetoothScannerWidgets::slot_reScan()
{   
    if (m_discoveryAgent->isActive() ||
            m_localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff)
        return;

    qDebug("Bluetooth: start scan");
    m_refreshButton->startAnimation();
    m_table->clearTable();
    clearDiscoveredDevices();
    m_discoveryAgent->start();

    if (firstEnter) {
        // when first enter application discovered devices become 0,
        // so clean again when scan first.
        qDebug("scan later.");
        QTimer::singleShot(500, this, SLOT(slot_cleanScanFirst()));
        firstEnter = false;
    }
}

void BluetoothScannerWidgets::slot_cleanScanFirst()
{
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    m_refreshButton->startAnimation();
    m_table->clearTable();
    clearDiscoveredDevices();
    m_discoveryAgent->start();
}

void BluetoothScannerWidgets::slot_paringDone(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    QList<QTableWidgetItem*> items = m_table->findItems(address.toString(), Qt::MatchContains);

    if (pairing == QBluetoothLocalDevice::Paired ||
            pairing == QBluetoothLocalDevice::AuthorizedPaired ) {
        for (int var = 0; var < items.count(); ++var) {
            m_bluez5Helper->connectDevice(address);
            if (m_bluez5Helper->isDeviceConnected(address))
                m_table->setItemState(items.at(var)->row(), BT_STATE_CONNECTED);
            else
                m_table->setItemState(items.at(var)->row(), BT_STATE_PAIRED);
        }
    }else{
        for (int var = 0; var < items.count(); ++var)
            m_table->setItemState(items.at(var)->row(), BT_STATE_NULL);
    }
    m_table->sortTable();
}

void BluetoothScannerWidgets::slot_onDeviceConnected(QBluetoothAddress address)
{
    qDebug("Bluetooth: device connected: %s", address.toString().toLatin1().data());
    QList<QTableWidgetItem*> items = m_table->findItems(address.toString(), Qt::MatchContains);
    for (int var = 0; var < items.count(); ++var)
        m_table->setItemState(items.at(var)->row(), BT_STATE_CONNECTED);

    m_table->sortTable();
}

void BluetoothScannerWidgets::slot_onDeviceDisconnected(QBluetoothAddress address)
{
    qDebug("Bluetooth: device disconnected: %s", address.toString().toLatin1().data());
    QList<QTableWidgetItem*> items = m_table->findItems(address.toString(), Qt::MatchContains);

    QBluetoothLocalDevice::Pairing pairing = m_localDevice->pairingStatus(address);
    if (pairing == QBluetoothLocalDevice::Paired ||
            pairing == QBluetoothLocalDevice::AuthorizedPaired ) {
        for (int var = 0; var < items.count(); ++var){
            m_table->setItemState(items.at(var)->row(), BT_STATE_PAIRED);
        }
    }else{
        for (int var = 0; var < items.count(); ++var)
            m_table->setItemState(items.at(var)->row(), BT_STATE_NULL);
    }

    m_table->sortTable();
}

void BluetoothScannerWidgets::slot_hostModeStateChanged(QBluetoothLocalDevice::HostMode mode)
{
    qDebug("Bluetooth: slot_hostModeStateChanged: %d", mode);
    switch (mode) {
    case QBluetoothLocalDevice::HostPoweredOff:
        m_table->clearTable();
        m_refreshButton->stopAnimation();
        if (m_discoveryAgent->isActive()) {
            qDebug("stop discovery");
            m_discoveryAgent->stop();
        }
        break;
    case QBluetoothLocalDevice::HostConnectable:
    case QBluetoothLocalDevice::HostDiscoverable:
    case QBluetoothLocalDevice::HostDiscoverableLimitedInquiry:
        slot_reScan();
        break;
    default:
        break;
    }
}

void BluetoothScannerWidgets::showEvent(QShowEvent *event)
{
    BaseWidget::showEvent(event);

    bool isBluetoothOpen = m_localDevice->hostMode() != QBluetoothLocalDevice::HostPoweredOff;
    m_switchHeader->setChecked(isBluetoothOpen);
    m_switchHeader->setTitle(m_localDevice->name());

    if (!isBluetoothOpen) {
        m_table->clearTable();
        m_refreshButton->stopAnimation();
        if (m_discoveryAgent->isActive())
            m_discoveryAgent->stop();
    }
}
