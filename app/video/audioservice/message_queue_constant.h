#ifndef AUDIO_MESSAGE_QUEUE_H
#define AUDIO_MESSAGE_QUEUE_H

/*
 * Three is two message queue in application.
 * one for send request for audio contorl to service and one
 * for receive audio state from service.
 */
#define RCV_QUEUE_KEY 666
#define REQ_QUEUE_KET 888
#define REQ_FEEDBACK_QUEUE_KEY 999

#define MSG_QUEUE_FLAG 0666

#define MSG_BUFF_LEN 500

/* Receive service audio state from service */
#define RCV_TYPE_MEDIA_STATE_CHANGED 1
#define RCV_TYPE_ERROR 2
#define RCV_TYPE_STATE_CHANGED 3
#define RCV_TYPE_META_DATA_AVAILABLE 4
#define RCV_TYPE_POSITION_CHANGE 5
#define RCV_TYPE_DURATION_CHANGE 6

/* Requeset type that send to service for audio control */
#define REQ_TYPE_SET_MEDIA 1
#define REQ_TYPE_PLAY 2
#define REQ_TYPE_GET_STATE 3
#define REQ_TYPE_PAUSE 4
#define REQ_TYPE_STOP 5
#define REQ_TYPE_GET_DURATION 6
#define REQ_TYPE_SET_POSITION 7
#define REQ_TYPE_GET_POSITION 8
#define REQ_TYPE_SET_VOLUME 9
#define REQ_TYPE_GET_VOLUME 10
#define REQ_TYPE_GET_MEDIA 11
#define REQ_TYPE_GET_MEDIA_TITLE 12
#define REQ_TYPE_GET_MEDIA_ARTIST 13
#define REQ_TYPE_CONNECT_STATE_CHANGE 14
#define REQ_TYPE_PLAY_MODE_CHANGED 15

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

/**
 * From service to client.
 */
struct state_message
{
    long int msg_type;
    long long value;
};

/**
 * Message for audio play control.
 */
struct control_message
{
    long int msg_type;
    long long intValue;
    char textValue[MSG_BUFF_LEN];
};

#endif // AUDIO_MESSAGE_QUEUE_H
