///////////////////////////////////////////////////////////
//  MediaNotificationSender.cpp
//  Implementation of the Class MediaNotificationSender
///////////////////////////////////////////////////////////

#include "MediaNotificationSender.h"
#include <sys/msg.h>
#include <QDebug>
#include <QThread>

#include <QtConcurrent/QtConcurrent>

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <unistd.h>

MediaNotificationSender::MediaNotificationSender(){
    isTermimalCalled = false;
    QtConcurrent::run(this ,&MediaNotificationSender::wait);
}



MediaNotificationSender::~MediaNotificationSender(){
    qDebug()<<"~MediaNotificationSender()";
}

void MediaNotificationSender::closeSocket()
{
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    isTermimalCalled = true;
}

void MediaNotificationSender::wait(){
    int new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;
    int flag = 1, len = sizeof(int);

    //建立TCP套接口
    //AF_INET: Internet IP Protocol
    //SOCK_STREAM: Sequenced, reliable, connection-based byte streams
    //0: IPPROTO_IP = 0, Dummy protocol for TCP
    if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        printf("create socket error");
        perror("socket");
        return;
    }

    ////初始化sockaddr_in结构体（地址和通道），并绑定端口
    my_addr.sin_family = AF_INET;
    //host byte order to net
    my_addr.sin_port = htons(PORT);
    //INADDR_ANY: Address to accept any incoming messages
    my_addr.sin_addr.s_addr = INADDR_ANY;
    //#define sin_zero __pad
    bzero(&(my_addr.sin_zero),8);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, len);

    ////绑定套接口
    if(bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
    {
        perror("bind socket error");
        return;
    }

    ////创建监听套接口
    //N connection requests will be queued before further requests are refused.
    if(listen(sockfd,10)==-1)
    {
        perror("listen");
        return;
    }

    qDebug("------reuse !!accpet start----------");
    ////等待连接
    while(!isTermimalCalled)
    {
        sin_size = sizeof(struct sockaddr); //either sockaddr or sockaddr_in can work normally

        printf("server is run.\n");

        ////如果建立连接，将产生一个全新的套接字
        if((new_fd = accept(sockfd,(struct sockaddr *)&their_addr,&sin_size))==-1)
        {
            perror("accept");
            break;
        }
        printf("accept success.\n");
        mutex.lock();

        m_clientList.append(new_fd);
	QList<int>::const_iterator iterator;
	for (iterator=m_clientList.constBegin(); iterator!=m_clientList.constEnd(); ++iterator) {
		struct tcp_info info;
		int len=sizeof(info);
		getsockopt(*iterator, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
		if((info.tcpi_state != TCP_ESTABLISHED)){
			close(*iterator);
			m_clientList.removeOne(*iterator);
			printf("close unuse accept socket fd.\n");
		}
	}
        mutex.unlock();

    }
}

void MediaNotificationSender::sendNotification(MediaNotification *notification){
    qDebug()<<"sendNotification:"<<notification->data.path<<"type:"<<notification->event;

    mutex.lock();
    qDebug()<<"clientList:"<<m_clientList.size();
    QList<int>::const_iterator iterator;
    for(iterator=m_clientList.constBegin(); iterator!=m_clientList.constEnd(); ++iterator){

        QByteArray ba;
        ba.resize(sizeof(MediaNotification)); //设置容量
        memcpy(ba.data(),notification,sizeof(MediaNotification));

        struct tcp_info info;
        int len=sizeof(info);
        getsockopt(*iterator, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
        if((info.tcpi_state==TCP_ESTABLISHED)){
            if(send(*iterator,notification,sizeof(MediaNotification),0)==-1){
                perror("send");
            }
        }else{
	    close(*iterator);
            m_clientList.removeOne(*iterator);
        }

    }

    mutex.unlock();
}

void MediaNotificationSender::newConnect()
{
    //    qDebug()<<"newConnect()";
}
