#include "imagecontroler.h"

#include <QVBoxLayout>

#ifdef DEVICE_EVB
int controler_button_width = 110;
int controler_bottom_height  = 140;
#else
int controler_button_width = 60;
int controler_bottom_height  = 70;
#endif

ImageControler::ImageControler(QWidget *parent) : BaseWidget(parent)
{
    setFixedHeight(controler_bottom_height);
    initLayout();
}

void ImageControler::initLayout()
{
    // button and it's layout
    m_btnLast = new FlatButton(this);
    m_btnLast->setFixedSize(controler_button_width, controler_button_width);
    m_btnLast->setStyleSheet("QPushButton{border-image:url(:/image/gallery/ic_last_normal.png);}"
//                             "QPushButton::hover{border-image:url(:/image/gallery/ic_last_hover.png);}"
                             "QPushButton::pressed{border-image:url(:/image/gallery/ic_last_pressed.png);}");

    m_btnZoomOut = new FlatButton(this);
    m_btnZoomOut->setFixedSize(controler_button_width, controler_button_width);
    m_btnZoomOut->setStyleSheet("QPushButton{border-image:url(:/image/gallery/ic_zoomout_normal.png);}"
//                                "QPushButton::hover{border-image:url(:/image/gallery/ic_zoomout_hover.png);}"
                                "QPushButton::pressed{border-image:url(:/image/gallery/ic_zoomout_pressed.png);}");

    m_btnZoomIn = new FlatButton(this);
    m_btnZoomIn->setFixedSize(controler_button_width, controler_button_width);
    m_btnZoomIn->setStyleSheet("QPushButton{border-image:url(:/image/gallery/ic_zoomin_normal.png);}"
//                               "QPushButton::hover{border-image:url(:/image/gallery/ic_zoomin_hover.png);}"
                               "QPushButton::pressed{border-image:url(:/image/gallery/ic_zoomin_pressed.png);}");

    m_btnRotate = new FlatButton(this);
    m_btnRotate->setFixedSize(controler_button_width, controler_button_width);
    m_btnRotate->setStyleSheet("QPushButton{border-image:url(:/image/gallery/ic_rotate_normal.png);}"
//                               "QPushButton::hover{border-image:url(:/image/gallery/ic_rotate_hover.png);}"
                               "QPushButton::pressed{border-image:url(:/image/gallery/ic_rotate_pressed.png);}");

    m_btnDetail = new FlatButton(this);
    m_btnDetail->setFixedSize(controler_button_width, controler_button_width);
    m_btnDetail->setStyleSheet("QPushButton{border-image:url(:/image/gallery/ic_detail_normal.png);}"
//                               "QPushButton::hover{border-image:url(:/image/gallery/ic_detail_hover.png);}"
                               "QPushButton::pressed{border-image:url(:/image/gallery/ic_detail_pressed.png);}");

    m_btnDelete = new FlatButton(this);
    m_btnDelete->setFixedSize(controler_button_width,controler_button_width);
    m_btnDelete->setStyleSheet("QPushButton{border-image:url(:/image/gallery/ic_delete_normal.png);}"
//                               "QPushButton::hover{border-image:url(:/image/gallery/ic_delete_hover.png);}"
                               "QPushButton::pressed{border-image:url(:/image/gallery/ic_delete_pressed.png);}");

    m_btnNext = new FlatButton(this);
    m_btnNext->setFixedSize(controler_button_width, controler_button_width);
    m_btnNext->setStyleSheet("QPushButton{border-image:url(:/image/gallery/ic_next_normal.png);}"
//                             "QPushButton::hover{border-image:url(:/image/gallery/ic_next_hover.png);}"
                             "QPushButton::pressed{border-image:url(:/image/gallery/ic_next_pressed.png);}");


    QHBoxLayout *buttonLayout = new QHBoxLayout;

    buttonLayout->addStretch(0);
    buttonLayout->addWidget(m_btnLast);
    buttonLayout->addWidget(m_btnZoomOut);
    buttonLayout->addWidget(m_btnZoomIn);
    buttonLayout->addWidget(m_btnRotate);
    buttonLayout->addWidget(m_btnDetail);
    buttonLayout->addWidget(m_btnDelete);
    buttonLayout->addWidget(m_btnNext);

    buttonLayout->addStretch(0);
    buttonLayout->setSpacing(0);

    setLayout(buttonLayout);
}
