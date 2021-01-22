#include "mainwindow.h"
#include "constant.h"

#include <QVBoxLayout>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>

const QString MUSIC_SEARCH_PATH_SDCARD = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/sdcard");
const QString MUSIC_SEARCH_PATH_UDISK = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/udisk");
const QString MUSIC_SEARCH_PATH_USERDATA = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/userdata");
const QString MUSIC_SEARCH_PATH_OEM = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/oem");
const QString MUSIC_SEARCH_PATH_MEDIA = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/media");
const QString MUSIC_SEARCH_PATH_MNT = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/mnt");

MainWindow::MainWindow(QWidget *parent) : BaseWindow(parent)
  , mediaHasUpdate(false)
  , m_mediaUpdateThread(0)
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

    // Start media source update thread.
    // Uevent for usb and inotify for file modify.
    m_mediaUpdateReceiver = new MediaNotificationReceiver();
    m_mediaUpdateReceiver->receive();

    m_mediaUpdateThread = new MediaUpdateThread(this);
}

void MainWindow::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_musicWid = new MusicWidgets(this);

    mainLayout->addWidget(m_musicWid);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);
}

void MainWindow::initConnection()
{
    connect(this, SIGNAL(beginUpdateMediaResource()), this, SLOT(slot_setUpdateFlag()));
    connect(this, SIGNAL(searchResultAvailable(QFileInfoList)),this, SLOT(slot_handleSearchResult(QFileInfoList)));
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

void MainWindow::slot_handleSearchResult(QFileInfoList musicFileList)
{
    m_musicWid->updateUiByRes(musicFileList);
}

void MainWindow::disableApplication()
{
    qDebug("disable music application");
    this->setVisible(false);
}

void MainWindow::enableApplication()
{
    qDebug("enable music application");
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

    this->close();
}

MusicWidgets* MainWindow::getMusicWidget()
{
    return m_musicWid;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_VolumeDown:
        m_musicWid->updateVolume(false);
        break;
    case Qt::Key_VolumeUp:
        m_musicWid->updateVolume(true);
        break;
    default:
        break;
    }
}

MainWindow::~MainWindow()
{
}

MediaUpdateThread::MediaUpdateThread(MainWindow *parent) : QThread(parent)
{
    m_parent = parent;

    m_searchSuffixList.append("mp3");
    m_searchSuffixList.append("wave");
    m_searchSuffixList.append("wma");
    m_searchSuffixList.append("ogg");
    m_searchSuffixList.append("midi");
    m_searchSuffixList.append("ra");
    m_searchSuffixList.append("mod");
    m_searchSuffixList.append("mp1");
    m_searchSuffixList.append("mp2");
    m_searchSuffixList.append("wav");
    m_searchSuffixList.append("flac");
    m_searchSuffixList.append("aac");
    m_searchSuffixList.append("m4a");

    qRegisterMetaType<QFileInfoList>("QFileInfoList");
}

void MediaUpdateThread::waitForThreadFinished()
{
    requestInterruption();
    quit();
    wait();
}

QFileInfoList MediaUpdateThread::findMusicFiles(const QString &path)
{
    QFileInfoList musicFiles;

    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext() && !isInterruptionRequested()) {
        QString name = it.next();
        QFileInfo info(name);
        if (info.isDir()) {
            musicFiles.append(findMusicFiles(name));
        }
        else{
            for (int i = 0; i < m_searchSuffixList.count(); i++) {
                if (info.suffix().compare(m_searchSuffixList.at(i), Qt::CaseInsensitive) == 0)
                    musicFiles.append(info);
            }
        }
    }
    return musicFiles;
}

void MediaUpdateThread::run()
{
    QFileInfoList musicFileList = findMusicFiles(MUSIC_SEARCH_PATH_SDCARD);
    musicFileList.append(findMusicFiles(MUSIC_SEARCH_PATH_UDISK));
    musicFileList.append(findMusicFiles(MUSIC_SEARCH_PATH_USERDATA));
    musicFileList.append(findMusicFiles(MUSIC_SEARCH_PATH_OEM));
    musicFileList.append(findMusicFiles(MUSIC_SEARCH_PATH_MEDIA));
    musicFileList.append(findMusicFiles(MUSIC_SEARCH_PATH_MNT));
    if (!isInterruptionRequested())
        emit m_parent->searchResultAvailable(musicFileList);
}
