#ifndef APPSCANNER_H
#define APPSCANNER_H
#include <QList>
#include "application.h"
class AppScanner
{
public:
    AppScanner();
public:
    QList<Application*>* scan(QString dir);
};

#endif // APPSCANNER_H
