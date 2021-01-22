#include "listheader.h"
#include "constant.h"

#include <QHBoxLayout>
#include <QMouseEvent>

#ifdef DEVICE_EVB
int video_header_height = 70;
#else
int video_header_height = 50;
#endif

ListHeader::ListHeader(QWidget *parent) : BaseWidget(parent)
{
    initWidget();

    connect(m_button1, SIGNAL(buttonClick()), this, SLOT(slot_onButtonLocalClick()));
//    connect(m_button2, SIGNAL(buttonClick()), this, SLOT(slot_onButtonNetClick()));
    m_button1->setFousedStyle();
}

void ListHeader::initWidget()
{
    QHBoxLayout *hmainlyout = new QHBoxLayout;

    m_button1 = new FuntionButton(tr("Local Video"), this);
//    m_button2 = new FuntionButton(tr("Net Video"), this);
    m_button1->setFixedHeight(video_header_height);
//    m_button2->setFixedHeight(video_header_height);

    hmainlyout->addWidget(m_button1);
//    hmainlyout->addWidget(m_button2);
    hmainlyout->setMargin(0);
    hmainlyout->setSpacing(0);

    setLayout(hmainlyout);
}

void ListHeader::slot_onButtonLocalClick()
{
//    m_button2->removeFouseStyle();
//    emit buttonLocalClick();
}

void ListHeader::slot_onButtonNetClick()
{
    m_button1->removeFouseStyle();
    emit buttonNetClick();
}

FuntionButton::FuntionButton(QString title, QWidget *parent) : BaseWidget(parent)
  , m_isCurItem(false)
{
    initWidget();

    m_title->setText(title);
}

void FuntionButton::initWidget()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;

    m_title = new QLabel(this);
    BaseWidget::setWidgetFontSize(m_title, font_size_big);
    BaseWidget::setWidgetFontBold(m_title);
    m_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_title->adjustSize();

    m_bottomLine = new QFrame(this);
    m_bottomLine->setFixedHeight(2);
    m_bottomLine->setStyleSheet("QFrame{border:1px solid rgb(100,100,100,255);}");
    m_bottomLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    vmainlyout->addWidget(m_title);
    vmainlyout->addWidget(m_bottomLine);
    vmainlyout->setMargin(0);
    vmainlyout->setSpacing(0);

    setLayout(vmainlyout);
}

void FuntionButton::enterEvent(QEvent *)
{
    m_title->setStyleSheet("color:rgb(26,158,255);");
    update();
}

void FuntionButton::leaveEvent(QEvent *)
{
    if (!m_isCurItem) {
        m_title->setStyleSheet("color:rgb(255,255,255);");
        update();
    }
}

void FuntionButton::mousePressEvent(QMouseEvent *event)
{
    if (!m_isCurItem && event->button() == Qt::LeftButton) {
        setFousedStyle();
        emit buttonClick();
    }
}

void FuntionButton::removeFouseStyle()
{
    m_isCurItem = false;
    m_title->setStyleSheet("color:rgb(255,255,255);");
    m_bottomLine->setStyleSheet("QFrame{border:1px solid rgb(100,100,100,255);}");

    update();
}

void FuntionButton::setFousedStyle()
{
    m_isCurItem = true;
    m_title->setStyleSheet("color:rgb(26,158,255);");
    m_bottomLine->setStyleSheet("QFrame{border:1px solid rgb(26,158,255);}");

    update();
}
