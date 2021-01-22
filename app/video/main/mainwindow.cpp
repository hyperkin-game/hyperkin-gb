#include "mainwindow.h"
#include "constant.h"
#include "player/videoinfoutil.h"

#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QApplication>

const QString VIDEO_SEARCH_PATH_SDCARD = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/sdcard");
const QString VIDEO_SEARCH_PATH_UDISK = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/udisk");
const QString VIDEO_SEARCH_PATH_USERDATA = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/userdata");
const QString VIDEO_SEARCH_PATH_OEM = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/oem");
const QString VIDEO_SEARCH_PATH_MEDIA = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/media");
const QString VIDEO_SEARCH_PATH_MNT = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/mnt");

MainWindow::MainWindow(QWidget *parent) : BaseWindow(parent),
    mediaHasUpdate(false),
    m_mediaUpdateThread(0)
{
    initData();
    initLayout();
    initConnection();

    slot_updateMedia();
}

void MainWindow::initData()
{
    setStyleSheet("QPushButton:pressed{padding:2px;background:rgb(204,228,247)}");

    // initialize global main class of 'MainWindow' for other widgets invokes.
    mainWindow = this;

    // start media source update thread.
    // uevent for usb and inotify for file modify.
    m_mediaUpdateReceiver = new MediaNotificationReceiver();
    m_mediaUpdateReceiver->receive();

    m_mediaUpdateThread = new MediaUpdateThread(this);
}

void MainWindow::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_videoWid = new VideoWidgets(this);

    mainLayout->addWidget(m_videoWid);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);
}

void MainWindow::initConnection()
{
    connect(this, SIGNAL(beginUpdateMediaResource()), this, SLOT(slot_setUpdateFlag()));
    connect(this, SIGNAL(searchResultAvailable(QFileInfoList)), this, SLOT(slot_updateUiByRes(QFileInfoList)));
    connect(m_mediaUpdateReceiver, SIGNAL(mediaNotification(MediaNotification*)), this, SLOT(slot_setUpdateFlag()));
}

void MainWindow::slot_setUpdateFlag()
{
    /*
     * This operation setted because that inotify event send no more one siganl.
     * So set a 500ms duration to ignore theres no-use siganls.
     * Note: it is expected to optimize.
     */
    if (!mediaHasUpdate) {
        mediaHasUpdate = true;
        QTimer::singleShot(500, this, SLOT(slot_updateMedia()));
    }
}

void MainWindow::slot_updateMedia()
{
    if (m_mediaUpdateThread->isRunning()) {
        mediaHasUpdate = false;
        return;
    }

    qDebug("Update media resource.");
    m_mediaUpdateThread->start();
    mediaHasUpdate = false;
}

void MainWindow::slot_updateUiByRes(QFileInfoList videoFileList)
{
    m_videoWid->updateUiByRes(videoFileList);
}

void MainWindow::disableApplication()
{
    qDebug("disable video application.");
    m_videoWid->setPlayerPause();
    this->setVisible(false);
}

void MainWindow::enableApplication()
{
    qDebug("enable video application.");
    this->setVisible(true);
}

void MainWindow::exitApplication()
{
    if (m_mediaUpdateReceiver) {
        delete m_mediaUpdateReceiver;
        m_mediaUpdateReceiver = 0;
    }

    if (m_mediaUpdateThread->isRunning())
        m_mediaUpdateThread->waitForThreadFinished();

    qApp->exit(0);
}

VideoWidgets* MainWindow::getVideoWidget()
{
    return m_videoWid;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_VolumeDown:
        m_videoWid->updateVolume(false);;
        break;
    case Qt::Key_VolumeUp:
        m_videoWid->updateVolume(true);
        break;
    case Qt::Key_PowerOff:
        m_videoWid->setPlayerPause();
        break;
    default:
        break;
    }
}

MediaUpdateThread::MediaUpdateThread(MainWindow *mainWindow) : QThread(mainWindow)
{
    m_mainWindow = mainWindow;

    m_searchSuffixList.append("mp4");
    m_searchSuffixList.append("avi");
    m_searchSuffixList.append("rm");
    m_searchSuffixList.append("rmvb");
    m_searchSuffixList.append("wmv");
    m_searchSuffixList.append("mkv");
    m_searchSuffixList.append("asf");
    m_searchSuffixList.append("mov");
    m_searchSuffixList.append("ts");
    m_searchSuffixList.append("mpg");
    m_searchSuffixList.append("mpg");
    m_searchSuffixList.append("m2ts");
    m_searchSuffixList.append("trp");
    m_searchSuffixList.append("flv");
    m_searchSuffixList.append("WEBM");
    m_searchSuffixList.append("3GP");
    m_searchSuffixList.append("Vob");
    m_searchSuffixList.append("MPG");
    m_searchSuffixList.append("tp");

    qRegisterMetaType<QFileInfoList>("QFileInfoList");
}

void MediaUpdateThread::waitForThreadFinished()
{
    requestInterruption();
    quit();
    wait();
}

QFileInfoList MediaUpdateThread::findVideoFiles(const QString &path)
{
    QFileInfoList videoFiles;

    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext() && !isInterruptionRequested()) {
        QString name = it.next();
        QFileInfo info(name);
        if (info.isDir()) {
            videoFiles.append(findVideoFiles(name));
        } else {
            for (int i = 0; i < m_searchSuffixList.count(); i++) {
                if (info.suffix().compare(m_searchSuffixList.at(i), Qt::CaseInsensitive) == 0)
                    videoFiles.append(info);
            }
        }
    }

    return videoFiles;
}

void MediaUpdateThread::run()
{
    QFileInfoList videoFileList = findVideoFiles(VIDEO_SEARCH_PATH_SDCARD);
    videoFileList.append(findVideoFiles(VIDEO_SEARCH_PATH_UDISK));
    videoFileList.append(findVideoFiles(VIDEO_SEARCH_PATH_USERDATA));
    videoFileList.append(findVideoFiles(VIDEO_SEARCH_PATH_OEM));
    videoFileList.append(findVideoFiles(VIDEO_SEARCH_PATH_MEDIA));
    videoFileList.append(findVideoFiles(VIDEO_SEARCH_PATH_MNT));
    if (!isInterruptionRequested())
        emit m_mainWindow->searchResultAvailable(videoFileList);

    QMap<QString, bool> *map = m_mainWindow->getVideoWidget()->getResolutionMap();
    for (int i = 0; i < videoFileList.size() && !isInterruptionRequested(); i++) {
        QString filePath = videoFileList.at(i).absoluteFilePath();
        if (!map->contains(filePath)) {
            bool isSuitable = VideoInfoUtil::isVideoSolutionSuitable(filePath);
            map->insert(filePath, isSuitable);
        }
    }
}
