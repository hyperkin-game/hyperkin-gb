#ifndef APPINFOREADER_H
#define APPINFOREADER_H
#include "application.h"
#include <QList>
#include <QJsonObject>
class AppInfoReader
{
public:
    AppInfoReader();
public:
    QList<Application*>* readFromPath(QString settings);
    Application* fromJsonObject(QByteArray byteArray);
    QList<Application*>* loadFromJsonArray(QByteArray byteArray);
    Application* loadFromJsonObject(QJsonObject &jsonObject);

};

#endif // APPINFOREADER_H
