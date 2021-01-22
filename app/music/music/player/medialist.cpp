#include "medialist.h"

#include <QTime>

MediaList::MediaList(QObject *parent) : QObject(parent)
  , m_currentIndex(-1)
{
    m_list.empty();

    setPlayMode(PlayInOrder);
}

void MediaList::setPlayMode(PlayMode playmode)
{
    m_playmode = playmode;
}

int MediaList::getPlayMode()
{
    return m_playmode;
}

void MediaList::changePlayMode()
{
    switch (m_playmode) {
    case PlayInOrder:
        m_playmode = PlayRandom;
        break;
    case PlayRandom:
        m_playmode = PlayOneCircle;
        break;
    case PlayOneCircle:
        m_playmode = PlayInOrder;
        break;
    }
}

void MediaList::removeItem(int index)
{
    m_list.removeAt(index);
}

QString MediaList::getPathAt(int index)
{
    if (m_list.isEmpty())
        return QString("");

    m_currentIndex = index;
    return m_list.value(index);
}

QString MediaList::getNextSongPath()
{
    switch (m_playmode) {
    case PlayOneCircle:
        break;
    case PlayInOrder:
        if (m_currentIndex + 1 >= m_list.count())
            m_currentIndex = 0;
        else
            m_currentIndex++;
        break;
    case PlayRandom: {
        QTime time = QTime::currentTime();
        qsrand(time.msec() + time.second() * 1000);
        int xxx = qrand() % m_list.count();
        m_currentIndex = xxx;
        break;
    }
    default:
        break;
    }

    return getPathAt(m_currentIndex);
}

QString MediaList::getPreSongPath()
{
    switch (m_playmode) {
    case PlayOneCircle:
        break;
    case PlayInOrder:
        if (m_currentIndex == 0)
            m_currentIndex = 0;
        else
            m_currentIndex--;
        break;
    case PlayRandom: {
        QTime time = QTime::currentTime();
        qsrand(time.msec() + time.second() * 1000);
        int xxx = qrand() % m_list.count();
        m_currentIndex = xxx;
        break;
    }
    default:
        break;
    }
    return getPathAt(m_currentIndex);
}

void MediaList::clearList()
{
    m_list.clear();
    m_currentIndex=0;
}
