#include "lyricutil.h"

#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QTextCodec>

bool LyricUtil::analyzeLrcContent(const QString &lrcFilePath)
{
    m_lrcMap.clear();

    QFile lrcFile(lrcFilePath);
    if (!lrcFile.exists())
        return false;

    lrcFile.open(QIODevice::ReadOnly);
    QTextStream lrcStream(&lrcFile);
    lrcStream.setAutoDetectUnicode(true);

    QRegExp timeExp;
    timeExp.setPatternSyntax(QRegExp::RegExp);
    timeExp.setCaseSensitivity(Qt::CaseSensitive);
    timeExp.setPattern("\\[([0-9]{2}):([0-9]{2})\\.([0-9]{2})\\]");

    // traverse file stream to initialize lrc map.
    while (!lrcStream.atEnd()) {
        QString line = lrcStream.readLine();
        int ret = timeExp.indexIn(line);
        QList<QTime> ticks;
        int lastindex = 0;
        while (ret >= 0) {
            QStringList tstr = timeExp.capturedTexts();
            QTime time(0, tstr[1].toInt(), tstr[2].toInt(), tstr[3].toInt());
            ticks.append(time);
            lastindex = ret + timeExp.matchedLength();
            ret = timeExp.indexIn(line, lastindex);
        }
        QString lyricStr = line.right(line.size() - lastindex);
        for (const QTime& t : ticks) {
            m_lrcMap.insert(t.minute() * 60 * 1000 + t.second() * 1000 + t.msec(), lyricStr);
        }
    }
    qDebug("lyric analyze complete with map size: %d", m_lrcMap.size());
    lrcFile.close();

    return m_lrcMap.size() > 0;
}

int LyricUtil::getCount()
{
    return m_lrcMap.keys().count();
}

int LyricUtil::getIndex(qint64 position)
{
    qint64 lt = 0;
    qint64 rt = m_lrcMap.values().count();
    qint64 mid = 0;
    while (lt < rt - 1) {
        mid = (lt + rt) / 2;
        if (m_lrcMap.keys().value(mid) > position)
            rt = mid;
        else
            lt = mid;
    }

    return lt;
}

QString LyricUtil::getLineAt(int index)
{
    return index < 0 || index >= m_lrcMap.size() ? "" : m_lrcMap.values().at(index);
}

void LyricUtil::clear()
{
    m_lrcMap.clear();
}
