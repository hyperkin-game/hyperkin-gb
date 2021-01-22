#include "gallerywidgets.h"

#include <QDebug>
#include <QCryptographicHash>
#include <QDir>

GalleryWidgets::GalleryWidgets(QWidget *parent) : BaseWidget(parent)
  , m_loadImageThread(0)
{
    setStyleSheet("QLabel{color:white;}");

    initLayout();
    initConnection();
    initData();
}

GalleryWidgets::~GalleryWidgets()
{
}

void GalleryWidgets::initData()
{
    // start an new thread to load image.
    m_loadImageThread = new LoadImageThread(this, m_middleWid);
}

void GalleryWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;

    m_topWid = new TopWidget(this);
    m_topWid->updateTopTitle(tr("imageGallery"));

    m_middleWid = new MiddleWidget(this);

    vmainlyout->addWidget(m_topWid);
    vmainlyout->addWidget(m_middleWid);
    vmainlyout->setMargin(0);
    vmainlyout->setSpacing(0);

    setLayout(vmainlyout);
}

void GalleryWidgets::initConnection()
{
    connect(m_topWid, SIGNAL(returnClicked()), this, SLOT(slot_onReturnClicked()));
    connect(m_middleWid, SIGNAL(viewerResChanged(QString)), this, SLOT(slot_onViewerResChanged(QString)));
    connect(this, SIGNAL(loadImageComplete()), this, SLOT(slot_onImagesResChanged()));
}

void GalleryWidgets::processFileList(QFileInfoList fileInfoList)
{
    if (m_loadImageThread->isRunning())
        return;

    m_loadImageThread->setFileInfoList(fileInfoList);
    m_loadImageThread->start();
}

void GalleryWidgets::removeImage(QString imagePath)
{
    if (m_imagesRes.keys().contains(imagePath)) {
        QMap<QString, QImage*>::iterator it = m_imagesRes.find(imagePath);
        delete it.value();
        m_imagesRes.remove(imagePath);
    }
}

void GalleryWidgets::slot_onImagesResChanged()
{
    emit m_middleWid->imagesResChanged();
}

void GalleryWidgets::slot_onReturnClicked()
{
    if (m_middleWid->isViewerMode()) {
        m_middleWid->leaveViewerMode();
        m_topWid->updateTopTitle(tr("imageGallery"));
    } else {
        // destory load-image thread while exit application.
        if (m_loadImageThread && m_loadImageThread->isRunning())
            m_loadImageThread->stopThread();

        mainWindow->exitApplication();
    }
}

void GalleryWidgets::slot_onViewerResChanged(QString imagePath)
{
    if (imagePath == "") {
        m_topWid->updateTopTitle(tr("imageGallery"));
    } else {
        QFileInfo *info = new QFileInfo(imagePath);
        if (info->exists())
            m_topWid->updateTopTitle(info->fileName());
    }
}

LoadImageThread::LoadImageThread(GalleryWidgets *parentWidget, MiddleWidget *middleWid)
    : QThread(parentWidget)
{
    m_parent = parentWidget;
    m_middleWid = middleWid;

    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    qRegisterMetaType<QMap<QString, QImage>>("QMap<QString, QImage*>");
}

void LoadImageThread::setFileInfoList(QFileInfoList infoList)
{
    m_infoList = infoList;
}

void LoadImageThread::stopThread()
{
    requestInterruption();
    quit();
    wait();
}

void LoadImageThread::run()
{
    QMap<QString, QImage*> &imagesRes = m_parent->getImagesRes();

    QList<QString> filePathList;
    for (int i = 0; i < m_infoList.size(); i++)
        filePathList.append(m_infoList.at(i).absoluteFilePath());

    // traverse map to delete non-existent item.
    QMap<QString, QImage*>::Iterator it;
    if (!filePathList.empty()) {
        for (it = imagesRes.begin(); it != imagesRes.end(); ) {
            if (!filePathList.contains(it.key())) {
                emit m_middleWid->sig_imagesResRemove(it.key());
                delete it.value();
                imagesRes.erase(it++);
            } else{
                it++;
            }
        }
    } else {
        for (it = imagesRes.begin(); it != imagesRes.end(); it++) {
            emit m_middleWid->sig_imagesResRemove(it.key());
            delete it.value();
        }
        imagesRes.clear();
    }

    /* traverse path list to add item.
       Note: creat thumb image first for view.*/
    QDir thumbDir("/tmp/thumb/");
    if (!thumbDir.exists())
        thumbDir.mkdir(thumbDir.absolutePath());

    QImage *tempImage;
    for (int i = 0; i < filePathList.size() && !isInterruptionRequested(); i++) {
        if (!imagesRes.keys().contains(filePathList.at(i))) {
            tempImage = new QImage();
            QString src_path = filePathList.at(i);
            QFileInfo thumb(thumbDir.absolutePath() + "/" + fileMD5(src_path) + ".thumb");

            if (!thumb.exists())
                compressImg(src_path, thumb.absoluteFilePath());

            if (tempImage->load(thumb.absoluteFilePath())) {
                imagesRes.insert(filePathList.at(i), tempImage);
                emit m_middleWid->sig_imagesResInsert(filePathList.at(i), tempImage);
                if (imagesRes.size() % 20 == 0) {
                    QThread::msleep(500);
                }
            } else {
                delete tempImage;
            }
        }
    }

    emit m_parent->loadImageComplete();
}

QString LoadImageThread::fileMD5(QString path)
{
    QString md5;
    QByteArray bb = QCryptographicHash::hash(path.toLatin1(), QCryptographicHash::Md5);
    md5.append(bb.toHex());

    return md5;
}

void LoadImageThread::compressImg(QString src, QString out)
{
    bool ret;
    QImage img;

    ret = img.load(src);
    if (!ret) {
        qDebug("image load failed for src: %s", src.toLocal8Bit().data());
        return;
    }

    QImage result = img.scaled(800, 600, Qt::KeepAspectRatio, Qt::FastTransformation).
            scaled(260, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ret = result.save(out, "JPEG", 100);
    if (!ret)
        qDebug() << "save image fail!";

    QFile file(out);
    qint64 fsz = file.size();

    int quality = 100;
    while (fsz > 2048) {
        quality = quality - 5;
        ret = result.save(out, "JPEG", quality);
        if (!ret)
            qDebug() << "save image fail! quality=" << quality;

        fsz = file.size();
        if (quality <= 0)
            break;
    }
}
