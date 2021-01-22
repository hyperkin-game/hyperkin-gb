///////////////////////////////////////////////////////////
//  MediaMonitor.h
//  Implementation of the Class MediaMonitor
///////////////////////////////////////////////////////////

#if !defined(EA_DE18BCB7_4D55_49e7_8206_B73D9C18D0A6__INCLUDED_)
#define EA_DE18BCB7_4D55_49e7_8206_B73D9C18D0A6__INCLUDED_

#include "MediaNotificationSender.h"
#include <list>
#include <QThread>

class MonitorThread:public QThread
{
    Q_OBJECT
public:
    MonitorThread(QObject *parent=0);
    ~MonitorThread(){}
    void setMediaNotificationSender(MediaNotificationSender*);
    void stopThread();
protected:
    MediaNotificationSender* m_sender;
    void run();
signals:
    void monitorEvent(MediaNotification* notification);
};


class MediaMonitor:public QThread
{
    Q_OBJECT
public:
    MediaMonitor();
    virtual ~MediaMonitor(){}

    void stopThread();
    MediaNotificationSender *m_MediaNotificationSender;

    void addFile(FileData* file);
    void listen();

    void removeFile(FileData* file);

private:
    std::list<FileData*> listenFiles;
    MonitorThread   *m_monitorThread;

public slots:
    void relisten(int);

};


#endif // !defined(EA_DE18BCB7_4D55_49e7_8206_B73D9C18D0A6__INCLUDED_)
