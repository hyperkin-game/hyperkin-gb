#ifndef LYRICUTIL_H
#define LYRICUTIL_H

#include <QMap>

class LyricUtil
{
public:
    LyricUtil(){}

    bool analyzeLrcContent(const QString &lrcFilePath);
    int getCount();
    int getIndex(qint64 pos);
    QString getLineAt(int index);
    void clear();

private:
    QMap<qint64, QString> m_lrcMap;
};

#endif // LYRICUTIL_H
