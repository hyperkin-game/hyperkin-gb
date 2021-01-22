///////////////////////////////////////////////////////////
//  MediaNotificationReceiver.h
//  Implementation of the Class MediaNotificationReceiver
///////////////////////////////////////////////////////////

#if !defined(EA_403C3F56_DF61_4ded_A00E_12E74A3A14C4__INCLUDED_)
#define EA_403C3F56_DF61_4ded_A00E_12E74A3A14C4__INCLUDED_

#include <QThread>
#include <QTcpSocket>
#include <QUdpSocket>

#include "MediaNotification.h"

#define PORT 9999

class MediaNotificationReceiver : public QObject
{
    Q_OBJECT
public:
    MediaNotificationReceiver();
    virtual ~MediaNotificationReceiver();

    void receive();

private:
    QTcpSocket *m_tcpSocket;
    QUdpSocket *m_udbSocket;

public slots:
    void onConnected();
    void readMesg();

signals:
    void mediaNotification(MediaNotification* notification);
};

#endif // !defined(EA_403C3F56_DF61_4ded_A00E_12E74A3A14C4__INCLUDED_)
