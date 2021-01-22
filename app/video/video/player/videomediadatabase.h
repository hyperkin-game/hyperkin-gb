/*#ifndef VIDEOMEDIADATABASE_H
#define VIDEOMEDIADATABASE_H

#include <QString>
#include <QVector>
#include <QStringList>

class videoMediaDataBase
{
public:
    explicit videoMediaDataBase(){}

    static void connectVideoInfo();
    static void addVideo(const QString &videoName, const QString &path, const QString &duration);
    static void removeVideo(const QString &videoName, const QString &path);
    static QVector<QStringList> getVideoInfo();
};

#endif // VIDEOMEDIADATABASE_H*/
