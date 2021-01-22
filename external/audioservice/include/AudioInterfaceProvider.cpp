#include "AudioInterfaceProvider.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

AudioInterfaceProvider::AudioInterfaceProvider()
{
    initMessageQueue();
}

void AudioInterfaceProvider::initMessageQueue()
{
    sendQueueId = msgget(REQ_QUEUE_KET,MSG_QUEUE_FLAG | IPC_CREAT);
    if (sendQueueId == -1)
        printf("AudioInterface: send queue init with error %s\n", strerror(errno));

    feedbackQueueId = msgget(REQ_FEEDBACK_QUEUE_KEY, MSG_QUEUE_FLAG | IPC_CREAT);
    if (feedbackQueueId == -1)
        printf("AudioInterface: request feedback init with error %s\n", strerror(errno));
}

void AudioInterfaceProvider::sendMessage(long int msgType, int intValue, char *textValue)
{
    if (sendQueueId != -1) {
        struct control_message message;
        message.msg_type = msgType;
        message.intValue = intValue;
        if (textValue != NULL)
            strcpy(message.textValue, textValue);
        else
            strcpy(message.textValue, "");
        //printf("AudioInterface: send message with type: %ld,intValue:%lld,textValue: %s\n", message.msg_type, message.intValue, message.textValue);

        if (msgsnd(sendQueueId, (void*)&message, MSG_BUFF_LEN + sizeof(long long), IPC_NOWAIT) == -1)
            printf("AudioInterface: send message failed with message type: %ld\n", msgType);
    }
}

bool AudioInterfaceProvider::sendMessageForResult(long int msgType, control_message *message,
                                                  int intValue, char *textValue)
{
    m_mtx.lock();
    if (sendQueueId != -1 && feedbackQueueId != -1)
        sendMessage(msgType, intValue, textValue);

    while (true) {
        if (msgrcv(feedbackQueueId, (void*)message, MSG_BUFF_LEN + sizeof(long long), 0, 0) != -1) {
            if (message->msg_type == msgType) {
                m_mtx.unlock();
                return true;
            }
        } else {
            printf("for result with error: %s\n", strerror(errno));
            m_mtx.unlock();
            return false;
        }
    }
}

void AudioInterfaceProvider::setMedia(char *filePath)
{
    sendMessage(REQ_TYPE_SET_MEDIA, 0, filePath);
}

void AudioInterfaceProvider::play()
{
    sendMessage(REQ_TYPE_PLAY);
}

void AudioInterfaceProvider::pause()
{
    sendMessage(REQ_TYPE_PAUSE);
}

void AudioInterfaceProvider::stop()
{
    sendMessage(REQ_TYPE_STOP);
}

int AudioInterfaceProvider::state()
{
    int state = 0;
    control_message message;
    memset(&message, 0, sizeof(message));
    if (sendMessageForResult(REQ_TYPE_GET_STATE, &message)) {
        state = message.intValue;
    }

    return state;
}

long long AudioInterfaceProvider::duration()
{
    long long duration = -1;
    control_message message;
    memset(&message, 0, sizeof(message));
    if (sendMessageForResult(REQ_TYPE_GET_DURATION, &message)) {
        duration = message.intValue;
    }

    return duration;
}

void AudioInterfaceProvider::setVolume(int value)
{
    sendMessage(REQ_TYPE_SET_VOLUME, value);
}

int AudioInterfaceProvider::volume()
{
    int volume = 0;
    control_message message;
    memset(&message, 0, sizeof(message));
    if (sendMessageForResult(REQ_TYPE_GET_VOLUME, &message)) {
        volume = message.intValue;
    }

    return volume;
}

void AudioInterfaceProvider::setPosition(long long pos)
{
    sendMessage(REQ_TYPE_SET_POSITION, pos);
}

long long AudioInterfaceProvider::position()
{
    long long position = 0;
    control_message message;
    memset(&message, 0, sizeof(message));
    if (sendMessageForResult(REQ_TYPE_GET_POSITION, &message)) {
        position = message.intValue;
    }

    return position;
}

char* AudioInterfaceProvider::currentMedia()
{
    char *mediaPath = new char[MSG_BUFF_LEN];
    control_message message;
    memset(&message, 0, sizeof(message));
    if (sendMessageForResult(REQ_TYPE_GET_MEDIA, &message)) {
        strcpy(mediaPath,message.textValue);
    }

    return mediaPath;
}

char* AudioInterfaceProvider::getArtist()
{
    char *mediaArtist = new char[MSG_BUFF_LEN];
    control_message message;
    memset(&message, 0, sizeof(message));
    if (sendMessageForResult(REQ_TYPE_GET_MEDIA_ARTIST, &message)) {
        strcpy(mediaArtist,message.textValue);
    }

    return mediaArtist;
}

char* AudioInterfaceProvider::getTitle()
{
    char *mediaTitle = new char[MSG_BUFF_LEN];
    control_message message;
    memset(&message, 0, sizeof(message));
    if (sendMessageForResult(REQ_TYPE_GET_MEDIA_TITLE, &message)) {
        strcpy(mediaTitle,message.textValue);
    }

    return mediaTitle;
}

void AudioInterfaceProvider::clientConnectStateChanged(bool isConnected)
{
    sendMessage(REQ_TYPE_CONNECT_STATE_CHANGE, isConnected ? 1 : 0);
}

void AudioInterfaceProvider::currentPlayModeChanged(int currentPlayMode)
{
    sendMessage(REQ_TYPE_PLAY_MODE_CHANGED, currentPlayMode);
}
