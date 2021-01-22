#include "message-helper.h"
#include "log.h"

#include <sys/msg.h>
#include <string.h>

static MessageHelper *gInstance = NULL;

MessageHelper::MessageHelper()
    : m_clientConnected(false)
{
    initMessageQueue();
}

MessageHelper* MessageHelper::getInstance()
{
    if (gInstance != NULL)
        return gInstance;

    gInstance = new MessageHelper();
    return gInstance;
}

void MessageHelper::initMessageQueue()
{
    m_sendMsgid = msgget(RCV_QUEUE_KEY, MSG_QUEUE_FLAG | IPC_CREAT);
    if (m_sendMsgid == -1) {
        Log::error("Init m_sendMsgid with error %s\n", strerror(errno));
    }

    m_recvMsgid = msgget(REQ_QUEUE_KET, MSG_QUEUE_FLAG | IPC_CREAT);
    if (m_recvMsgid == -1) {
        Log::error("Init m_recvMsgid with error %s\n", strerror(errno));
    }

    m_fbMsgid = msgget(REQ_FEEDBACK_QUEUE_KEY, MSG_QUEUE_FLAG | IPC_CREAT);
    if (m_fbMsgid == -1) {
        Log::error("Init m_fbMsgid with error %s\n", strerror(errno));
    }
}

void MessageHelper::onClientStateChanged(bool isConnected)
{
    m_clientConnected = isConnected;
}

bool MessageHelper::isClientConnected()
{
    return m_clientConnected;
}

void MessageHelper::sendMessage(long int msgType, long long value)
{
    struct state_message data;

    if (m_sendMsgid != -1 && m_clientConnected) {
        data.msg_type = msgType;
        data.value = value;
        if (msgsnd(m_sendMsgid, (void*)&data, sizeof(long long), IPC_NOWAIT) == -1)
            Log::error("Failed to send message with msg_type: %ld\n", msgType);
    }
}

bool MessageHelper::receiveMessage(control_message *data)
{
    if (m_recvMsgid != -1) {
        if (msgrcv(m_recvMsgid, (void*)data, MSG_BUFF_LEN + sizeof(long long), 0, 0) != -1)
            return true;
    }

    return false;
}

void MessageHelper::sendFeedbackMessage(long int msgType, long long intValue , const char *textValue)
{
    struct control_message data;

    if (m_fbMsgid != -1 && m_clientConnected) {
        data.msg_type = msgType;
        data.intValue = intValue;

        if (textValue != NULL)
            strcpy(data.textValue, textValue);
        else
            strcpy(data.textValue, "");

        if (msgsnd(m_fbMsgid, (void*)&data, MSG_BUFF_LEN + sizeof(long long), IPC_NOWAIT) == -1)
            Log::error("Failed to send feedback message with msg_type: %ld\n", msgType);
    }
}
