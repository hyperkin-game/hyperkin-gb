///////////////////////////////////////////////////////////
//  MediaNotificationReceiver.cpp
//  Implementation of the Class MediaNotificationReceiver
///////////////////////////////////////////////////////////

#include "MediaNotificationReceiver.h"
#include "MediaNotification.h"
#include <QDebug>

MediaNotificationReceiver::MediaNotificationReceiver()
{
    qDebug() << "MediaNotificationReceiver()";
}

MediaNotificationReceiver::~MediaNotificationReceiver()
{
    qDebug() << "~MediaNotificationReceiver()";
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->close();
        delete m_tcpSocket;
        m_tcpSocket = 0;
    }
}

void MediaNotificationReceiver::receive()
{
    qDebug() << "receive()";
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readMesg()));
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    m_tcpSocket->connectToHost("127.0.0.1", PORT);
}

void MediaNotificationReceiver::readMesg()
{
    qDebug() << "MediaNotificationReceiver::readMesg()";
    QByteArray qba = m_tcpSocket->readAll();
    MediaNotification* notification = (MediaNotification*)qba.data();
    qba.data();
    qDebug() << "client receive:" << notification->data.path << " type:" << notification->event;

    emit mediaNotification(notification);
}

void MediaNotificationReceiver::onConnected()
{
    qDebug() << "onConnected()";
}
