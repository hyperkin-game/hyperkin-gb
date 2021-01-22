#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QThread>

class MessageHandler : public QThread
{
    Q_OBJECT
public:
    MessageHandler(QObject *parent = 0);
    ~MessageHandler();

protected:
    void run();

private:
    int queueId;
    void initMessageQueue();

signals:
    void mediaStatusChanged(int);
    void stateChanged(int);
    void error(int);
    void metaDataAvailable();
    void positionChanged(long long);
    void durationChanged(long long);
};

#endif // MESSAGEHANDLER_H
