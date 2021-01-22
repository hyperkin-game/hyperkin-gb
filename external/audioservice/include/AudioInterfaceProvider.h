#ifndef __INTERFACE_PROVIDER_H__
#define __INTERFACE_PROVIDER_H__

#include "message_queue_constant.h"

#include <mutex>

class AudioInterfaceProvider
{
public:
    AudioInterfaceProvider();
    ~AudioInterfaceProvider(){}

    void play();
    void pause();
    void stop();

    void setMedia(char *filePath);
    char* currentMedia();
    char* getArtist();
    char* getTitle();

    long long position();
    void setPosition(long long pos);
    long long duration();

    int volume();
    void setVolume(int value);

    //bool isAudioAvailable();
    //bool isAvailable();

    int state();

    void clientConnectStateChanged(bool isConnected);
    void currentPlayModeChanged(int currentPlayMode);

private:
    std::mutex m_mtx;

    // sendQueueId: message queue for send request to service.
    // feedbackQueueId: message queu for receive value that feedback by service.
    int sendQueueId;
    int feedbackQueueId;

    void initMessageQueue();
    void sendMessage(long int msgType, int intValue = 0, char *textValue = 0);
    bool sendMessageForResult(long int msgType, control_message *message,
                              int intValue = 0, char *textValue = 0);
};

#endif // __INTERFACE_PROVIDER_H__
