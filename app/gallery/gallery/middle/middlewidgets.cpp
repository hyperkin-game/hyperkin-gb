#include "middlewidgets.h"
#include "constant.h"

MiddleWidget::MiddleWidget(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(43, 45, 51);

    initLayout();
    initConnection();
}

void MiddleWidget::initLayout()
{
    m_stackLayout = new QStackedLayout(this);

    m_emptyImgWid = new EmptyImagesWidget(this);
    m_thumbImgWid = new ThumbImageWidget(this);
    m_viewerWid = new ImageViewerWidget(this);

    m_stackLayout->addWidget(m_emptyImgWid);
    m_stackLayout->addWidget(m_thumbImgWid);
    m_stackLayout->addWidget(m_viewerWid);
    m_stackLayout->setCurrentWidget(m_thumbImgWid);

    setLayout(m_stackLayout);
}

void MiddleWidget::initConnection()
{
    connect(this, SIGNAL(imageEmpty()), this, SLOT(slot_showEmptyImageTip()));
    connect(this, SIGNAL(imageItemClick(QString)), this, SLOT(slot_showImageViewer(QString)));
    connect(this, SIGNAL(imagesResChanged()), this, SLOT(slot_onImagesResChanged()));
    connect(this, SIGNAL(sig_imagesResInsert(QString,QImage*)), this, SLOT(slot_imagesResInsert(QString,QImage*)));
    connect(this, SIGNAL(sig_imagesResRemove(QString)), this, SLOT(slot_imagesResRemove(QString)));
}

bool MiddleWidget::isViewerMode()
{
    return m_stackLayout->currentWidget() == (QWidget*) m_viewerWid;
}

void MiddleWidget::leaveViewerMode()
{
    m_stackLayout->setCurrentWidget(m_thumbImgWid);
}

void MiddleWidget::slot_showEmptyImageTip()
{
    emit viewerResChanged("");
    m_stackLayout->setCurrentWidget(m_emptyImgWid);
}

void MiddleWidget::slot_showImageViewer(QString imagePath)
{
    m_stackLayout->setCurrentWidget(m_viewerWid);
    m_viewerWid->updateRes(imagePath);
}

void MiddleWidget::slot_onImagesResChanged()
{
    QMap<QString, QImage*> &imagesRes = mainWindow->getGalleryWidget()->getImagesRes();
    if (imagesRes.size() > 0) {
        if (m_stackLayout->currentWidget() == m_emptyImgWid)
            m_stackLayout->setCurrentWidget(m_thumbImgWid);

        emit m_thumbImgWid->imagesResChanged();
        emit m_viewerWid->imagesResChanged(isViewerMode());
    } else {
        slot_showEmptyImageTip();
    }
}

void MiddleWidget::slot_imagesResInsert(QString path, QImage *img)
{
    m_thumbImgWid->onImagesResInsert(path, img);
}

void MiddleWidget::slot_imagesResRemove(QString path)
{
    m_thumbImgWid->onImagesResRemove(path);
}
