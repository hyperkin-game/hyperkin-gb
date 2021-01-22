#ifndef UEVENTTHREAD_H
#define UEVENTTHREAD_H

#include <QThread>

#define UEVENT_MSG_LEN 4096
struct luther_gliethttp {
    const char *action;
    const char *devPath;
    const char *gpioState;
};

class UeventThread:public QThread
{
    Q_OBJECT
public:
    UeventThread(QObject *parent=0);
    ~UeventThread(){}

    void stopThread();

private:
    void parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp);
protected:
    void run();
signals:
    void reverseTriggerStateChanged(bool triggered);
    void ueventDeviceChange(int);
};

#endif // UEVENTTHREAD_H
