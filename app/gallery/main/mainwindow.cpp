#include "mainwindow.h"
#include "MediaNotificationReceiver.h"

#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>

const QString GALLERY_SEARCH_PATH_SDCARD = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/sdcard");
const QString GALLERY_SEARCH_PATH_UDISK = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/udisk");
const QString GALLERY_SEARCH_PATH_USERDATA = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/userdata");
const QString GALLERY_SEARCH_PATH_OEM = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/oem");
const QString GALLERY_SEARCH_PATH_MEDIA = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/media");
const QString GALLERY_SEARCH_PATH_MNT = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/mnt");

MainWindow::MainWindow(QWidget *parent) : BaseWindow(parent)
  , mediaHasUpdate(false)
  , m_mediaUpdateThread(0)
{
    initData();
    initLayout();
    initConnection();

    slot_updateMedia();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initData()
{
    // initialize global main class of 'MainWindow' for other widgets invokes.
    mainWindow = this;

    m_mediaUpdateReceiver = new MediaNotificationReceiver();
    m_mediaUpdateReceiver->receive();

    m_mediaUpdateThread = new MediaUpdateThread(this);
}

void MainWindow::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_galleryWid = new GalleryWidgets(this);

    mainLayout->addWidget(m_galleryWid);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);
}

void MainWindow::initConnection()
{
    connect(this, SIGNAL(beginUpdateMediaResource()), this, SLOT(slot_setUpdateFlag()));
    connect(this, SIGNAL(searchResultAvailable(QFileInfoList)), this, SLOT(slot_handleSearchResult(QFileInfoList)));
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

void MainWindow::disableApplication()
{
    qDebug("disable gallery application");
    this->setVisible(false);
}

void MainWindow::enableApplication()
{
    qDebug("enable gallery application");
    this->setVisible(true);
}

void MainWindow::slot_handleSearchResult(QFileInfoList fileInfoList)
{
    m_galleryWid->processFileList(fileInfoList);
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

GalleryWidgets *MainWindow::getGalleryWidget()
{
    return m_galleryWid;
}

MediaUpdateThread::MediaUpdateThread(MainWindow *mainWindow) : QThread(mainWindow)
{
    m_mainWindow = mainWindow;

    m_searchSuffixList.append("jpg");
    m_searchSuffixList.append("png");
    m_searchSuffixList.append("bmp");
    m_searchSuffixList.append("jpeg");
    m_searchSuffixList.append("svg");
    m_searchSuffixList.append("titf");
    m_searchSuffixList.append("gif");

    qRegisterMetaType<QFileInfoList>("QFileInfoList");
}

void MediaUpdateThread::waitForThreadFinished()
{
    requestInterruption();
    quit();
    wait();
}

QFileInfoList MediaUpdateThread::findImgFiles(const QString &path)
{
    QFileInfoList imageFiles;
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    while (it.hasNext()) {
        QString name = it.next();
        QFileInfo info(name);
        if (info.isDir()) {
            imageFiles.append(findImgFiles(name));
        } else {
            for (int i = 0; i < m_searchSuffixList.count(); i++) {
                if (info.suffix().compare(m_searchSuffixList.at(i), Qt::CaseInsensitive) == 0)
                    imageFiles.append(info);
            }
        }
    }
    return imageFiles;
}

void MediaUpdateThread::run()
{
    QFileInfoList fileInfoList = findImgFiles(GALLERY_SEARCH_PATH_SDCARD);
    fileInfoList.append(findImgFiles(GALLERY_SEARCH_PATH_UDISK));
    fileInfoList.append(findImgFiles(GALLERY_SEARCH_PATH_USERDATA));
    fileInfoList.append(findImgFiles(GALLERY_SEARCH_PATH_OEM));
    fileInfoList.append(findImgFiles(GALLERY_SEARCH_PATH_MEDIA));
    fileInfoList.append(findImgFiles(GALLERY_SEARCH_PATH_MNT));
    if (!isInterruptionRequested())
        emit m_mainWindow->searchResultAvailable(fileInfoList);
}
