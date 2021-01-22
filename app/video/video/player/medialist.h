#ifndef VIDEOLIST_H
#define VIDEOLIST_H

#include <QObject>
#include <QUrl>

enum PlayMode
{
    PlayInOrder,
    PlayRandom,
    PlayOneCircle
};

/**
 * used for manager the video list.
 *
 * Each video item saved with url and you can add、remove
 * or get video item.
 */
class MediaList : public QObject
{
    Q_OBJECT
public:
    MediaList(QObject *parent = 0);

    void clearList();
    void setPlayMode(PlayMode);
    PlayMode getPlayMode();

    QUrl getUrlAt(int index);
    QUrl getNextVideoUrl();
    QUrl getPreVideoUrl();

    void removeItem(int index);
    void changePlayMode();

    inline void addPlayList(const QString &path)
    {
        m_list.append(QUrl::fromLocalFile(path));
    }

    QList<QUrl> getUrlList()
    {
        return m_list;
    }

private:
    int m_currentIndex;

    QList<QUrl> m_list;
    PlayMode m_playmode;
};

#endif // VIDEOLIST_H
