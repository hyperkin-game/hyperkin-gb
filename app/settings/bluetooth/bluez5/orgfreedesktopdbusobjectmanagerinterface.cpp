#include "orgfreedesktopdbusobjectmanagerinterface.h"

OrgFreedesktopDBusObjectManagerInterface::OrgFreedesktopDBusObjectManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgFreedesktopDBusObjectManagerInterface::~OrgFreedesktopDBusObjectManagerInterface()
{
}

