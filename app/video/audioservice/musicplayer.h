#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include "messagehandler.h"
#include "AudioInterfaceProvider.h"

class MusicPlayer : public QObject
{
    Q_OBJECT
public:
    MusicPlayer(QObject *parent = 0);
    ~MusicPlayer();

    enum State
    {
        StoppedState,
        PlayingState,
        PausedState
    };

    enum MediaStatus
    {
        UnknownMediaStatus,
        NoMedia,
        LoadingMedia,
        LoadedMedia,
        StalledMedia,
        BufferingMedia,
        BufferedMedia,
        EndOfMedia,
        InvalidMedia
    };

    enum Error
    {
        NoError,
        ResourceError,
        FormatError,
        NetworkError,
        AccessDeniedError,
        ServiceMissingError,
        MediaIsPlaylist
    };

    void play();
    void pause();
    void stop();

    void setMedia(const QString &filePath);
    QString currentMedia();
    QString getMediaTitle();
    QString getMediaArtist();

    qint64 position();
    void setPosition(qint64);
    qint64 duration();

    int volume();
    void setVolume(int value);

    bool isAudioAvailable();
    bool isAvailable();

    MusicPlayer::State state();

    void clientExit();
    void connectToService();

    void currentPlayModeChanged(int currentPlayMode);

private:
    AudioInterfaceProvider *provider;
    MessageHandler *m_messageHandler;

    void initConnection();

private slots:
    void slot_onMediaStatusChanged(int);
    void slot_onStateChanged(int);
    void slot_onError(int);

signals:
    void mediaStatusChanged(MusicPlayer::MediaStatus);
    void stateChanged(MusicPlayer::State);
    void error(MusicPlayer::Error);
    void metaDataAvailable();
    void positionChanged(qint64);
    void durationChanged(qint64);
};

#endif // MUSICPLAYER_H
