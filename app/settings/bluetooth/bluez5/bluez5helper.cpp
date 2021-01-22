#include "bluez5helper.h"
#include <QBluetoothLocalDevice>

/*!
    Finds the path for the local adapter with \a wantedAddress or an empty string
    if no local adapter with the given address can be found.
    If \a wantedAddress is \c null it returns the first/default adapter or an empty
    string if none is available.

    If \a ok is false the lookup was aborted due to a dbus error and this function
    returns an empty string.
 */
QString findAdapterForAddress(const QBluetoothAddress &wantedAddress, bool *ok = 0)
{
    OrgFreedesktopDBusObjectManagerInterface manager(QStringLiteral("org.bluez"),
                                                     QStringLiteral("/"),
                                                     QDBusConnection::systemBus());

    QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        if (ok)
            *ok = false;

        return QString();
    }

    typedef QPair<QString, QBluetoothAddress> AddressForPathType;
    QList<AddressForPathType> localAdapters;

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (iface == QStringLiteral("org.bluez.Adapter1")) {
                AddressForPathType pair;
                pair.first = path.path();
                pair.second = QBluetoothAddress(ifaceList.value(iface).value(
                                                    QStringLiteral("Address")).toString());
                if (!pair.second.isNull())
                    localAdapters.append(pair);
                break;
            }
        }
    }

    if (ok)
        *ok = true;

    if (localAdapters.isEmpty())
        return QString(); // -> no local adapter found

    if (wantedAddress.isNull())
        return localAdapters.front().first; // -> return first found adapter

    foreach (const AddressForPathType &pair, localAdapters) {
        if (pair.second == wantedAddress)
            return pair.first; // -> found local adapter with wanted address
    }

    return QString(); // nothing matching found
}

Bluez5Helper::Bluez5Helper(QObject *parent) : QObject(parent),
    device1Target(0),
    adapterBluez5(0),
    managerBluez5(0)
{
    registerQBluetoothLocalDeviceMetaType();
    initializeAdapterBluez5();
}

void Bluez5Helper::registerQBluetoothLocalDeviceMetaType()
{
    qRegisterMetaType<QBluetoothLocalDevice::HostMode>();
    qRegisterMetaType<QBluetoothLocalDevice::Pairing>();
    qRegisterMetaType<QBluetoothLocalDevice::Error>();

    qDBusRegisterMetaType<InterfaceList>();
    qDBusRegisterMetaType<ManagedObjectList>();
}

void Bluez5Helper::initializeAdapterBluez5()
{
    managerBluez5 = new OrgFreedesktopDBusObjectManagerInterface(
                QStringLiteral("org.bluez"),
                QStringLiteral("/"),
                QDBusConnection::systemBus(), this);

    bool ok = true;
    const QString adapterPath = findAdapterForAddress(QBluetoothAddress(), &ok);
    if (!ok || adapterPath.isEmpty()) {
        return;
    }
    adapterBluez5 = new OrgBluezAdapter1Interface(QStringLiteral("org.bluez"),
                                                  adapterPath,
                                                  QDBusConnection::systemBus(), this);
}

void Bluez5Helper::disconnectDevice(const QBluetoothAddress &targetAddress)
{
    qDebug("disconnectDevice device %s", targetAddress.toString().toLatin1().data());
    changeConnectState(targetAddress, false);
}

void Bluez5Helper::connectDevice(const QBluetoothAddress &targetAddress)
{
    qDebug("connect device %s", targetAddress.toString().toLatin1().data());
    changeConnectState(targetAddress, true);
}

void Bluez5Helper::changeConnectState(const QBluetoothAddress &targetAddress,bool connectIntent)
{
    if (device1Target) {
        delete device1Target;
        device1Target = 0;
    }

    QDBusPendingReply<ManagedObjectList> reply = managerBluez5->GetManagedObjects();
    if (reply.isError())
        return;

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (iface == QStringLiteral("org.bluez.Device1")) {
                OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                path.path(),
                                                QDBusConnection::systemBus());
                if (targetAddress == QBluetoothAddress(device.address())) {
                    device1Target = new OrgBluezDevice1Interface(QStringLiteral("org.bluez"), path.path(),
                                                                 QDBusConnection::systemBus(), this);
                    if (connectIntent)
                        device1Target->Connect();
                    else
                        device1Target->Disconnect();

                    return;
                }
            }
        }
    }
}

void Bluez5Helper::removeUnpairedDevice(const QBluetoothAddress &targetAddress)
{
    if(device1Target){
        delete device1Target;
        device1Target = 0;
    }

    QDBusPendingReply<ManagedObjectList> reply = managerBluez5->GetManagedObjects();
    if (reply.isError()) {
        return;
    }

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (iface == QStringLiteral("org.bluez.Device1")) {

                OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                path.path(),
                                                QDBusConnection::systemBus());
                if (targetAddress == QBluetoothAddress(device.address())) {
                    device1Target = new OrgBluezDevice1Interface(QStringLiteral("org.bluez"), path.path(),
                                                                 QDBusConnection::systemBus(), this);

                    if (adapterBluez5 && !device1Target->paired() && !device1Target->connected()) {
                        qDebug("remove device: %s",path.path().toLatin1().data());
                        adapterBluez5->RemoveDevice(QDBusObjectPath(path.path()));
                    }
                    return;
                }
            }
        }
    }
}

bool Bluez5Helper::isDeviceConnected(const QBluetoothAddress &targetAddress)
{
    if (device1Target) {
        delete device1Target;
        device1Target = 0;
    }

    QDBusPendingReply<ManagedObjectList> reply = managerBluez5->GetManagedObjects();
    if (reply.isError())
        return false;

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (iface == QStringLiteral("org.bluez.Device1")) {

                OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                path.path(),
                                                QDBusConnection::systemBus());
                if (targetAddress == QBluetoothAddress(device.address())) {
                    device1Target = new OrgBluezDevice1Interface(QStringLiteral("org.bluez"), path.path(),
                                                                 QDBusConnection::systemBus(), this);
                    return device1Target->connected();
                }
            }
        }
    }
    return false;
}
