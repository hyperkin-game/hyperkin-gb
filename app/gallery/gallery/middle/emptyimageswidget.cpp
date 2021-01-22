#include "emptyimageswidget.h"
#include "basepushbutton.h"
#include "constant.h"

#include <QVBoxLayout>
#include <QLabel>

#ifdef DEVICE_EVB
int center_image_width = 300;
#else
int center_image_width = 150;
#endif

EmptyImagesWidget::EmptyImagesWidget(QWidget *parent) : BaseWidget(parent)
{
    initLayout();
}

void EmptyImagesWidget::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;

    FlatButton *centerImage = new FlatButton(this);
    centerImage->setEnabled(false);
    centerImage->setFixedSize(center_image_width, center_image_width);
    centerImage->setBackgroundImage(":/image/gallery/ic_empty_image.png");

    QHBoxLayout *centerImageLyout = new QHBoxLayout;
    centerImageLyout->addStretch(0);
    centerImageLyout->addWidget(centerImage);
    centerImageLyout->addStretch(0);

    QLabel *tip1 = new QLabel(tr("All in the family"), this);
    tip1->setAlignment(Qt::AlignCenter);

    QLabel *tip2 = new QLabel(tr("Take a picture & Photo saved on this device appear here."), this);
    tip2->setAlignment(Qt::AlignCenter);

    vmainlyout->addStretch(0);
    vmainlyout->addLayout(centerImageLyout);
    vmainlyout->addWidget(tip1);
    vmainlyout->addWidget(tip2);
    vmainlyout->addSpacing(100);
    vmainlyout->addStretch(0);
    vmainlyout->setSpacing(15);

    // set the layout in the center of page
    QHBoxLayout *lyout = new QHBoxLayout;
    lyout->addStretch(1);
    lyout->addLayout(vmainlyout);
    lyout->addStretch(1);

    setLayout(lyout);
}
