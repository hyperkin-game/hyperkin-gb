#include "orgbluezdevice1interface.h"


OrgBluezDevice1Interface::OrgBluezDevice1Interface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgBluezDevice1Interface::~OrgBluezDevice1Interface()
{
}

