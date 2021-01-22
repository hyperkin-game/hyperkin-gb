#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gallerywidgets.h"
#include "base/basewindow.h"
#include "constant.h"
#include "MediaNotificationReceiver.h"

#include <QThread>
#include <QFileInfoList>

class GalleryWidgets;
class MediaUpdateThread;

class MainWindow : public BaseWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    GalleryWidgets* getGalleryWidget();
    void exitApplication();

protected:
    // used for disable or enable application when car-reverse event comes.
    void disableApplication();
    void enableApplication();

private:
    bool mediaHasUpdate;
    MediaUpdateThread *m_mediaUpdateThread;

    GalleryWidgets *m_galleryWid;
    MediaNotificationReceiver *m_mediaUpdateReceiver;

    void initData();
    void initLayout();
    void initConnection();

public slots:
    void slot_setUpdateFlag();
    void slot_updateMedia();
    void slot_handleSearchResult(QFileInfoList fileInfoList);

signals:
    void beginUpdateMediaResource();
    void searchResultAvailable(QFileInfoList fileInfoList);
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
    QFileInfoList findImgFiles(const QString &path);

protected:
    void run();
};

#endif // MAINWINDOW_H
