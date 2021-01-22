///////////////////////////////////////////////////////////
//  MediaMonitor.cpp
//  Implementation of the Class MediaMonitor
///////////////////////////////////////////////////////////

#include "MediaMonitor.h"
#include <string.h>
#include <QDebug>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <QDirIterator>

#define EVENT_NUM 12
#define SEARCH_PATH "/mnt"

char *event_str[EVENT_NUM] =
{
    (char*)"IN_ACCESS",
    (char*)"IN_MODIFY",
    (char*)"IN_ATTRIB",
    (char*)"IN_CLOSE_WRITE",
    (char*)"IN_CLOSE_NOWRITE",
    (char*)"IN_OPEN",
    (char*)"IN_MOVED_FROM",
    (char*)"IN_MOVED_TO",
    (char*)"IN_CREATE",
    (char*)"IN_DELETE",
    (char*)"IN_DELETE_SELF",
    (char*)"IN_MOVE_SELF"
};

QList<QString> getAllDirPath(const QString& path)
{
    QList<QString> dirPathList;
    QFileInfo pathInfo(path);
    if(pathInfo.isDir()){
        dirPathList.append(pathInfo.absoluteFilePath());
    }

    QDirIterator it(path,QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
    while (it.hasNext()){
        QString name = it.next();
        QFileInfo info(name);
        if (info.isDir()){
            if(!dirPathList.contains(info.absoluteFilePath())){
                dirPathList.append(info.absoluteFilePath());
                dirPathList.append(getAllDirPath(name));
            }
        }
    }
    return dirPathList;
}


MonitorThread::MonitorThread(QObject *parent):QThread(parent){

}

void MonitorThread::stopThread(){
    requestInterruption();
    terminate();
    quit();
    wait();
}

void MonitorThread::setMediaNotificationSender(MediaNotificationSender *sender){
    m_sender=sender;
}

void MonitorThread::run(){
    qDebug()<<"MonitorThread run";
    int fd;
    int len;
    int nread;
    char buf[BUFSIZ];
    struct inotify_event *event;
    int i;

    fd = inotify_init();
    if( fd < 0 ){
        qDebug("inotify_init failed\n");
        return;
    }

    QList<QString> pathList = getAllDirPath(SEARCH_PATH);
    for(int i=0;i<pathList.size();i++){
        qDebug("Add inotify path: %s",pathList.at(i).toLatin1().data());
        inotify_add_watch(fd, pathList.at(i).toLatin1().data(),
                          IN_CREATE | IN_DELETE | IN_DELETE_SELF);
    }

    inotify_add_watch(fd, SEARCH_PATH,
                      IN_CREATE | IN_DELETE | IN_DELETE_SELF );

    buf[sizeof(buf) - 1] = 0;
    while( (len = read(fd, buf, sizeof(buf) - 1)) > 0 && !isInterruptionRequested()){
        nread = 0;
        while( len > 0 ){
            event = (struct inotify_event *)&buf[nread];
            for(i=0; i<EVENT_NUM; i++){
                if((event->mask >> i) & 1){
                    if(event->len > 0){
                        qDebug("%s --- %s\n", event->name, event_str[i]);
                        if(m_sender){
                            MediaNotification notification;
                            if(strcmp(event_str[i],"IN_CREATE")==0){
                                notification.event=FileAdd;
                            }else{
                                notification.event=FileRemove;
                            }
                            strcpy(notification.data.path,event->name);
                            notification.data.size=strlen(event->name);
                            if(isInterruptionRequested()){
                                break;
                            }
                            m_sender->sendNotification(&notification);
                        }
                    }
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }

    qDebug()<<"MonitorThread end";
}


MediaMonitor::MediaMonitor(){

    m_MediaNotificationSender= new MediaNotificationSender();
}



void MediaMonitor::stopThread(){
    m_MediaNotificationSender->closeSocket();
    delete m_MediaNotificationSender;

    m_monitorThread->stopThread();
}



void MediaMonitor::addFile(FileData* file){
    listenFiles.push_back(file);
}


void MediaMonitor::listen(){
    qDebug()<<"MediaMonitor::listen()";
    m_monitorThread = new MonitorThread();
    m_monitorThread ->setMediaNotificationSender(m_MediaNotificationSender);
    m_monitorThread ->start();
}


void MediaMonitor::relisten(int add){
    qDebug()<<"MediaMonitor::relisten()";
    if(m_monitorThread){
        m_monitorThread->requestInterruption();
    }
    if(m_MediaNotificationSender){
        MediaNotification notification;
        if(add){
            notification.event=DeviceAdd;
        }else{
            notification.event=DeviceRemove;
        }
        m_MediaNotificationSender->sendNotification(&notification);
    }
    m_monitorThread = new MonitorThread();
    m_monitorThread ->setMediaNotificationSender(m_MediaNotificationSender);
    m_monitorThread ->start();

}


void MediaMonitor::removeFile(FileData* file){

    std::list<FileData*>::iterator iter = listenFiles.begin();
    for (; iter != listenFiles.end(); )
    {
        if (strcmp(file->path,(*iter)->path) != 0)
        {
            ++iter;
        }
        else
        {
            listenFiles.erase(iter++);
        }
    }

}
