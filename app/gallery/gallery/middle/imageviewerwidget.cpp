#include "imageviewerwidget.h"
#include "cmessagebox.h"
#include "constant.h"
#include "imagedetailwidget.h"

#include <QVBoxLayout>

#ifdef DEVICE_EVB
int button_last_image_width = 100;
int layout_spacing = 50;
#else
int button_last_image_width = 40;
int layout_spacing = 30;
#endif

ImageViewerWidget::ImageViewerWidget(QWidget *parent) : BaseWidget(parent)
  , m_middleWidgets((MiddleWidget*)parent)
{
    setBackgroundColor(0, 0, 0);

    initLayout();
    initConnection();
}

void ImageViewerWidget::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_imageViewer = new ImageViewer(this);
    m_imageControler = new ImageControler(this);

    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_imageViewer);
    mainLayout->addWidget(m_imageControler);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(30);

    setLayout(mainLayout);
}

void ImageViewerWidget::initConnection()
{
    connect(this, SIGNAL(imagesResChanged(bool)), this, SLOT(slot_onImagesResChanged(bool)));

    connect(m_imageControler->m_btnLast, SIGNAL(clicked(bool)), this, SLOT(slot_lastImage()));
    connect(m_imageControler->m_btnNext, SIGNAL(clicked(bool)), this, SLOT(slot_nextImage()));
    connect(m_imageControler->m_btnZoomOut, SIGNAL(clicked(bool)), this, SLOT(slot_imageZoomOut()));
    connect(m_imageControler->m_btnZoomIn, SIGNAL(clicked(bool)), this, SLOT(slot_imageZoomIn()));
    connect(m_imageControler->m_btnRotate, SIGNAL(clicked(bool)), this, SLOT(slot_imageRotate()));
    connect(m_imageControler->m_btnDelete, SIGNAL(clicked(bool)), this, SLOT(slot_deleteImage()));
    connect(m_imageControler->m_btnDetail, SIGNAL(clicked(bool)), this, SLOT(slot_viewDetail()));
}

void ImageViewerWidget::updateRes(QString imagePath)
{
    if (imagePath.endsWith(QString("gif"), Qt::CaseInsensitive)) {
        m_imageViewer->setMoviePath(imagePath);
        m_imagePath = imagePath;
        emit m_middleWidgets->viewerResChanged(imagePath);
    } else {
        if (m_imageViewer->setPixmap(imagePath)) {
            m_imagePath = imagePath;
            emit m_middleWidgets->viewerResChanged(imagePath);
        }
    }
}

void ImageViewerWidget::slot_onImagesResChanged(bool update)
{
    QMap<QString, QImage*> &imagesRes = mainWindow->getGalleryWidget()->getImagesRes();
    if (!imagesRes.keys().contains(m_imagePath) && update) {
        QMap<QString, QImage*>::Iterator it = imagesRes.begin();
        updateRes(it.key());
    }
}

void ImageViewerWidget::slot_lastImage()
{
    QMap<QString, QImage*> &imagesRes = mainWindow->getGalleryWidget()->getImagesRes();
    QMap<QString, QImage*>::Iterator it = imagesRes.begin();
    while (it != imagesRes.end()) {
        if (it.key() == m_imagePath) {
            if (it != imagesRes.begin())
                --it;
            else
                it = imagesRes.end() - 1;
            updateRes(it.key());
            break;
        }
        ++it;
    }
}

void ImageViewerWidget::slot_nextImage()
{
    QMap<QString, QImage*> &imagesRes = mainWindow->getGalleryWidget()->getImagesRes();
    QMap<QString, QImage*>::Iterator it = imagesRes.begin();
    while (it != imagesRes.end()) {
        if (it.key() == m_imagePath) {
            if (it != (imagesRes.end() - 1))
                ++it;
            else
                it = imagesRes.begin();

            updateRes(it.key());
            break;
        }
        ++it;
    }
}

void ImageViewerWidget::slot_viewDetail()
{
    ImageDetailWidget::showImageDetail(this, m_imagePath, m_imageViewer->getSize());
}

void ImageViewerWidget::slot_deleteImage()
{
    QFileInfo *info = new QFileInfo(m_imagePath);
    if (info->exists()) {
        int result = CMessageBox::showCMessageBox(this, tr("Delete images?"), tr("Delete"), tr("Cancel"));
        if (result == CMessageBox::RESULT_CONFIRM) {
            QString removePath = m_imagePath;
            if (QFile::remove(removePath)) {
                slot_nextImage();
                mainWindow->getGalleryWidget()->removeImage(removePath);
                emit m_middleWidgets->sig_imagesResRemove(removePath);
            }
        }
    }
}

void ImageViewerWidget::slot_imageZoomOut()
{
    m_imageViewer->zoomOut();
}

void ImageViewerWidget::slot_imageZoomIn()
{
    m_imageViewer->zoomIn();
}

void ImageViewerWidget::slot_imageRotate()
{
    m_imageViewer->clockwise90();
}
