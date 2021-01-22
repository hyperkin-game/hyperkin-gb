#ifndef BLUETOOTHSCANNERWIDGETS_H
#define BLUETOOTHSCANNERWIDGETS_H

#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/qbluetoothglobal.h>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QLowEnergyController>
#include <QLabel>

#include "bluetoothdevicetable.h"
#include "base/basewidget.h"
#include "base/switchbutton.h"
#include "base/basepushbutton.h"
#include "bluez5/bluez5helper.h"

class BluetoothSwitchHeader : public BaseWidget
{
    Q_OBJECT
public:
    BluetoothSwitchHeader(QWidget *parent = 0);

    void setChecked(bool checked);
    void setTitle(const QString &title);

private:
    QLabel *m_switchText;
    SwitchButton *m_switchButton;

signals:
    void sig_checkedChanged(bool);
};

class BluetoothScannerWidgets : public BaseWidget
{
    Q_OBJECT
public:
    BluetoothScannerWidgets(QWidget *parent = 0);
    ~BluetoothScannerWidgets(){}

private:
    BluetoothSwitchHeader *m_switchHeader;
    QLabel *availableTitle;
    bool firstEnter;

    QBluetoothLocalDevice *m_localDevice;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    Bluez5Helper *m_bluez5Helper;

    RotatableButton *m_refreshButton;
    BluetoothDeviceTable *m_table;

    void initData();
    void initLayout();
    void initConnection();
    void clearDiscoveredDevices();

protected:
    void showEvent(QShowEvent*);

private slots:
    void slot_addDevice(const QBluetoothDeviceInfo&);
    void slot_scanFinished();
    void slot_hostModeStateChanged(QBluetoothLocalDevice::HostMode);
    void slot_onBluetoothSwitchCheckChanged(bool);
    void slot_reScan();
    void slot_cleanScanFirst();
    void slot_paringDone(const QBluetoothAddress&, QBluetoothLocalDevice::Pairing);
    void slot_onDeviceConnected(QBluetoothAddress);
    void slot_onDeviceDisconnected(QBluetoothAddress);
    void slot_onTableItemClicked(int, int);
    void slot_onTableItemlongPressed(int);
    void retranslateUi();
};

#endif // BLUETOOTHSCANNERWIDGETS_H
