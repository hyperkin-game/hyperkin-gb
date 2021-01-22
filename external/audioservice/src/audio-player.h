#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#include <gst/gst.h>

#include "play-list.h"
#include "message-helper.h"

using namespace std;

class AudioPlayer
{
public:
    AudioPlayer(int argc, char *argv[]);
    ~AudioPlayer();

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

    GstElement* getPlaybin();
    void processEOS();
    void parseTagList(const GstTagList *tags);
    bool processBusMessage(GstMessage *message);

    void loadFromUri(string uri);
    bool play();
    bool pause();
    void stop();

    void updateList();
    void nextSong();
    void changePlayMode(int playMode);
    void onClientStateChanged(bool isConnected);

    void sendMediaName();

    gint64 position();
    void sendPosition();
    void updatePosition();
    bool setPosition(gint64 pos);

    gint64 getDuration();
    void updateDuration();
    void sendDuration();

    void sendState();

    void setVolume(int volume);
    void sendVolume();

    void sendArtist();
    void sendTitle();

private:
    GstElement* m_playbin;

    string m_uri;
    MessageHelper *m_msgHelper;
    AudioPlayer::State m_state;
    AudioPlayer::MediaStatus m_mediaStatus;

    gint64 m_duration;
    bool m_stopUpdatePosition;
    gint64 m_lastPosition;
    int m_volume;

    MediaList *m_list;
    GstTagList *m_tagList;
    string m_mediaArtist;
    string m_mediaTitle;
    string m_mediaLocation;
};

#endif // __AUDIO_PLAYER_H__
