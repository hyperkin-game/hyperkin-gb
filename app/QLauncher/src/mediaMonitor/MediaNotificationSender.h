///////////////////////////////////////////////////////////
//  MediaNotificationSender.h
//  Implementation of the Class MediaNotificationSender
///////////////////////////////////////////////////////////

#if !defined(EA_9235DDDA_B79F_43fd_B920_764147F36A15__INCLUDED_)
#define EA_9235DDDA_B79F_43fd_B920_764147F36A15__INCLUDED_

#include "MediaNotification.h"
#define MESSAGE_TYPE 5566


#define PORT 9999
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMutex>
class MediaNotificationSender:QObject
{
    Q_OBJECT
public:
    MediaNotificationSender();
    virtual ~MediaNotificationSender();

    void closeSocket();
    void wait();

private:
    bool isTermimalCalled;
    int msgid;
    int sockfd;
    QTcpServer *m_tcpServer;
    QList<int> m_clientList;
    QMutex mutex;

public:
    void sendNotification(MediaNotification *notification);

signals:
    void sig_sendNotification(MediaNotification *notification);

public slots:
    void newConnect();
};

#endif // !defined(EA_9235DDDA_B79F_43fd_B920_764147F36A15__INCLUDED_)
