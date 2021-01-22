#ifndef __MESSAGE_HELPER_H__
#define __MESSAGE_HELPER_H__

#include "message_queue_constant.h"

class MessageHelper
{
private:
    MessageHelper();
    ~MessageHelper() {}

public:
    static MessageHelper* getInstance();

    void sendMessage(long int msgType, long long value = 0);
    bool receiveMessage(control_message *data);
    void sendFeedbackMessage(long int msgType, long long intValue = 0, const char *textValue = 0);

    void onClientStateChanged(bool isConnected);
    bool isClientConnected();

private:
    /*
     * m_sendMsgid is for the message queue that send message to client proactive.
     * m_RecvMsgid is for the message queue that receive message from the client.
     * m_fbMsgid is for the message queue that the feedback to client's request.
     */
    int m_sendMsgid;
    int m_recvMsgid;
    int m_fbMsgid;

    bool m_clientConnected;

    void initMessageQueue();
};

#endif // __MESSAGE_HELPER_H__
