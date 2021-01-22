#include "orgbluezadapter1interface.h"

/*
 * Implementation of interface class OrgBluezAdapter1Interface
 */
OrgBluezAdapter1Interface::OrgBluezAdapter1Interface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgBluezAdapter1Interface::~OrgBluezAdapter1Interface()
{
}

