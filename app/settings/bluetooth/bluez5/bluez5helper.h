#ifndef BLUEZ5HELPER_H
#define BLUEZ5HELPER_H

#include <QBluetoothAddress>

#include "orgbluezdevice1interface.h"
#include "orgbluezadapter1interface.h"
#include "orgfreedesktopdbusobjectmanagerinterface.h"

class Bluez5Helper : public QObject
{
public:
    Bluez5Helper(QObject *parent);

    void disconnectDevice(const QBluetoothAddress &targetAddress);
    void connectDevice(const QBluetoothAddress &targetAddress);
    void removeUnpairedDevice(const QBluetoothAddress &targetAddress);
    bool isDeviceConnected(const QBluetoothAddress &targetAddress);

private:
    OrgBluezDevice1Interface *device1Target;
    OrgBluezAdapter1Interface *adapterBluez5;
    OrgFreedesktopDBusObjectManagerInterface *managerBluez5;

    void registerQBluetoothLocalDeviceMetaType();
    void initializeAdapterBluez5();
    void changeConnectState(const QBluetoothAddress &targetAddress, bool connectIntent);
};

#endif // BLUEZ5HELPER_H
