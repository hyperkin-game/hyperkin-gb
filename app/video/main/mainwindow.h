#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videowidgets.h"
#include "basewindow.h"
#include "MediaNotificationReceiver.h"

#include <QThread>
#include <QFileInfoList>

class MediaUpdateThread;

/**
 * The main window of application.
 *
 * Used for global control.such as keypress response、media update、
 * initial interface etc.
 */
class MainWindow : public BaseWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow(){}

    VideoWidgets* getVideoWidget();
    void exitApplication();

protected:
    void keyPressEvent(QKeyEvent *event);
    // used for disable or enable application when car-reverse event comes.
    void disableApplication();
    void enableApplication();

private:
    bool mediaHasUpdate;
    MediaUpdateThread *m_mediaUpdateThread;

    VideoWidgets *m_videoWid;
    // thread for media resource update.
    MediaNotificationReceiver *m_mediaUpdateReceiver;

    void initData();
    void initLayout();
    void initConnection();

signals:
    void beginUpdateMediaResource();
    void searchResultAvailable(QFileInfoList videoFileList);

public slots:
    void slot_updateMedia();
    void slot_setUpdateFlag();
    void slot_updateUiByRes(QFileInfoList videoFileList);
};

class MediaUpdateThread : public QThread
{
public:
    MediaUpdateThread(MainWindow *mainWindow);
    ~MediaUpdateThread(){}

    void waitForThreadFinished();

private:
    MainWindow *m_mainWindow;

    QList<QString> m_searchSuffixList;
    QFileInfoList findVideoFiles(const QString &path);

protected:
    void run();
};

#endif // MAINWINDOW_H
