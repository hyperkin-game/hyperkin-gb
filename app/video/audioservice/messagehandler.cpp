#include "messagehandler.h"
#include "message_queue_constant.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

MessageHandler::MessageHandler(QObject *parent) : QThread(parent)
{
    initMessageQueue();
}

void MessageHandler::initMessageQueue()
{
    queueId = msgget(RCV_QUEUE_KEY,MSG_QUEUE_FLAG | IPC_CREAT);
    if (queueId == -1)
        qDebug("init message receive queue with error %s", strerror(errno));
}

MessageHandler::~MessageHandler()
{
    requestInterruption();
    terminate();
    quit();
    wait();
}

void MessageHandler::run()
{
    struct state_message message;
    do {
        if (msgrcv(queueId, (void*)&message, sizeof(long long), 0, 0) != -1) {
            int messageValue = message.value;
            // qDebug("Receive message with type: %ld,value: %d", message.msg_type, messageValue);
            switch (message.msg_type) {
            case RCV_TYPE_MEDIA_STATE_CHANGED:
                emit mediaStatusChanged(messageValue);
                break;
            case RCV_TYPE_STATE_CHANGED:
                emit stateChanged(messageValue);
                break;
            case RCV_TYPE_ERROR:
                emit error(messageValue);
                break;
            case RCV_TYPE_META_DATA_AVAILABLE:
                emit metaDataAvailable();
                break;
            case RCV_TYPE_POSITION_CHANGE:
                emit positionChanged(messageValue);
                break;
            case RCV_TYPE_DURATION_CHANGE:
                emit durationChanged(messageValue);
                break;
            default:
                break;
            }
        }
    } while (!isInterruptionRequested());
}
