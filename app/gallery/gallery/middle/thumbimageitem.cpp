#include "thumbimageitem.h"

ThumbImageItem::ThumbImageItem(QString imagePath, QImage *image) : BaseWidget()
  , isChecked(false)
  , toUpdate(false)
  , m_imagePath(imagePath)
  , m_image(image)
  , imageMapper(new brightnessMapper)
{
    initLayout();
}

void ThumbImageItem::initLayout()
{
    m_stackedLayout = new QStackedLayout();

    thumbImage = new QLabel(this);

    // checkbox for flag that is selected..
    QVBoxLayout *m_statusLayout = new QVBoxLayout;

    m_checkImage = new QLabel(this);
    m_checkImage->setPixmap(QPixmap(":/image/gallery/ic_thumb_checked.png").scaled(40, 40));

    m_statusLayout->addStretch(0);
    m_statusLayout->addWidget(m_checkImage);

    QWidget *m_statusWid = new QWidget(this);
    m_statusWid->setLayout(m_statusLayout);

    m_stackedLayout->setStackingMode(QStackedLayout::StackAll);
    m_stackedLayout->addWidget(thumbImage);
    m_stackedLayout->addWidget(m_statusWid);
    m_stackedLayout->setCurrentIndex(0);

    setLayout(m_stackedLayout);
}

void ThumbImageItem::resizeEvent(QResizeEvent *)
{
    thumbImage->setPixmap(QPixmap::fromImage(*m_image).scaled(width(), height()));
}


void ThumbImageItem::paintEvent(QPaintEvent *)
{
    if (toUpdate) {
        if (isChecked) {
            imageMapper->updateBCG(0.5, 1, 1);
            thumbImage->setPixmap(QPixmap::fromImage(imageMapper->apply(*m_image)).scaled(width(), height()));
        } else {
            thumbImage->setPixmap(QPixmap::fromImage(*m_image).scaled(width(), height()));
        }
        toUpdate = false;
    }
}


void ThumbImageItem::onItemClick()
{
    if (isChecked) {
        m_stackedLayout->setCurrentIndex(0);
        isChecked = false;
    } else {
        m_stackedLayout->setCurrentIndex(1);
        isChecked = true;
    }

    toUpdate = true;
    update();
}

