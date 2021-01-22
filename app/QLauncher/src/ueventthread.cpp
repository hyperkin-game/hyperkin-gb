#include "ueventthread.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <QDebug>
#include <errno.h>

static int open_luther_gliethttp_socket(void);

UeventThread::UeventThread(QObject *parent):QThread(parent)
{
}

void UeventThread::stopThread()
{
    requestInterruption();
    terminate();
    quit();
    wait();
}

void UeventThread::run()
{
    int device_fd = -1;
    char msg[UEVENT_MSG_LEN+2];
    int n;

    device_fd = open_luther_gliethttp_socket();
    do {
        while((n = recv(device_fd, msg, UEVENT_MSG_LEN, 0)) > 0) {
            struct luther_gliethttp luther_gliethttp;

            if(n == UEVENT_MSG_LEN) /* overflow -- discard */
                continue;

            msg[n] = '\0';
            msg[n+1] = '\0';

            parse_event(msg, &luther_gliethttp);
        }
    } while(!isInterruptionRequested());
}

static int open_luther_gliethttp_socket(void)
{
    struct sockaddr_nl addr;
    int sz = 64*1024;
    int s;
    int val = 1;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = 0;
    addr.nl_groups = NETLINK_KOBJECT_UEVENT;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s < 0)
        return -1;

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        qDebug("bind error:%s\n", strerror(errno));
        close(s);
        return -1;
    }
    return s;
}

void UeventThread::parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp)
{
    luther_gliethttp->action = "";
    luther_gliethttp->devPath = "";
    luther_gliethttp->gpioState = "";

    while (*msg) {
        if (!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            luther_gliethttp->action = msg;
        } else if (!strncmp(msg, "DEVPATH=", 8)) {
            msg += 8;
            luther_gliethttp->devPath = msg;
        } else if(!strncmp(msg, "GPIO_NAME=", 10)){
            msg += 10;
            luther_gliethttp->gpioState = msg;
        }

        /* Advance to after the next \0 */
        while(*msg++);
    }

    if(strcmp(luther_gliethttp->action,"change")==0 && strstr(luther_gliethttp->devPath,"car-reverse")){
        if(strstr(luther_gliethttp->gpioState,"on")){
            emit reverseTriggerStateChanged(true);
        }else if(strstr(luther_gliethttp->gpioState,"over")){
            emit reverseTriggerStateChanged(false);
        }
    }

    QRegExp regExp;
    regExp.setPatternSyntax(QRegExp::RegExp);
    regExp.setCaseSensitivity(Qt::CaseSensitive);
    regExp.setPattern("sd[a-z]");

    QRegExp regExp2;
    regExp2.setPatternSyntax(QRegExp::RegExp);
    regExp2.setCaseSensitivity(Qt::CaseSensitive);
    regExp2.setPattern("mmcblk");


    if(strcmp(luther_gliethttp->action,"remove")==0 &&regExp.indexIn(QString(luther_gliethttp->devPath))>=0||
            strcmp(luther_gliethttp->action,"remove")==0 &&regExp2.indexIn(QString(luther_gliethttp->devPath))>=0||
            (strcmp(luther_gliethttp->action,"add")==0&&regExp.indexIn(QString(luther_gliethttp->devPath))>=0) ||
            (strcmp(luther_gliethttp->action,"add")==0&&regExp2.indexIn(QString(luther_gliethttp->devPath))>=0)){
        printf("event{'%s','%s'}\n",
               luther_gliethttp->action, luther_gliethttp->devPath);

        emit ueventDeviceChange(strcmp(luther_gliethttp->action,"add")==0);
        qDebug()<<"mmcblk or sd "<<luther_gliethttp->action<<"|" <<luther_gliethttp->devPath;

    }
}
