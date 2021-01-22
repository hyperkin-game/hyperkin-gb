#include "musicplayer.h"

MusicPlayer::MusicPlayer(QObject *parent) : QObject(parent)
{
    provider = new AudioInterfaceProvider();

    // create a message queue handler thread for service message.
    m_messageHandler = new MessageHandler(this);
    m_messageHandler->start();

    initConnection();
}

void MusicPlayer::initConnection()
{
    connect(m_messageHandler, SIGNAL(mediaStatusChanged(int)), this, SLOT(slot_onMediaStatusChanged(int)));
    connect(m_messageHandler, SIGNAL(stateChanged(int)), this, SLOT(slot_onStateChanged(int)));
    connect(m_messageHandler, SIGNAL(metaDataAvailable()), this, SIGNAL(metaDataAvailable()));
    connect(m_messageHandler, SIGNAL(error(int)), this, SLOT(slot_onError(int)));
    connect(m_messageHandler, SIGNAL(positionChanged(long long)), this, SIGNAL(positionChanged(qint64)));
    connect(m_messageHandler, SIGNAL(durationChanged(long long)), this, SIGNAL(durationChanged(qint64)));
}

void MusicPlayer::slot_onMediaStatusChanged(int newMediaState)
{
    emit mediaStatusChanged(MediaStatus(newMediaState));
}

void MusicPlayer::slot_onStateChanged(int newState)
{
    emit stateChanged(State(newState));
}

void MusicPlayer::slot_onError(int errorCode)
{
    emit error(Error(errorCode));
}

void MusicPlayer::play()
{
    provider->play();
}

void MusicPlayer::pause()
{
    provider->pause();
}

void MusicPlayer::stop()
{
    provider->stop();
}

void MusicPlayer::setMedia(const QString &filePath)
{
    provider->setMedia(filePath.toLocal8Bit().data());
}

QString MusicPlayer::currentMedia()
{
    char *mediaName = provider->currentMedia();
    if (mediaName != NULL)
        return QString::fromLocal8Bit(mediaName);
    else
        return QString("");
}

QString MusicPlayer::getMediaTitle()
{
    char *mediaTitle = provider->getTitle();
    if (mediaTitle != NULL)
        return QString::fromLocal8Bit(mediaTitle);
    else
        return QString("");
}

QString MusicPlayer::getMediaArtist()
{
    char *mediaArtist = provider->getArtist();
    if (mediaArtist != NULL)
        return QString::fromLocal8Bit(mediaArtist);
    else
        return QString("");
}

qint64 MusicPlayer::position()
{
    return provider->position();
}

void MusicPlayer::setPosition(qint64 position)
{
    provider->setPosition(position);
}

qint64 MusicPlayer::duration()
{
    return provider->duration();
}

int MusicPlayer::volume()
{
    return provider->volume();
}

void MusicPlayer::setVolume(int value)
{
    provider->setVolume(value);
}

bool MusicPlayer::isAudioAvailable()
{
    return true;
}

bool MusicPlayer::isAvailable()
{
    return true;
}

MusicPlayer::State MusicPlayer::state()
{
    return State(provider->state());
}

void MusicPlayer::clientExit()
{
    provider->clientConnectStateChanged(false);
}

void MusicPlayer::connectToService()
{
    provider->clientConnectStateChanged(true);
}

void MusicPlayer::currentPlayModeChanged(int currentPlayMode)
{
    provider->currentPlayModeChanged(currentPlayMode);
}
