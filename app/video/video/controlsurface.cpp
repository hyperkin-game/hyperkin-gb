#include "controlsurface.h"

#include <QHBoxLayout>

ControlSurface::ControlSurface(QWidget *parent) : BaseWidget(parent)
  , m_mediaOn(false)
{ 
    initLayout();
    initConnetion();

    slot_hideFurface();
}

void ControlSurface::initLayout()
{
    QVBoxLayout *vmainLayout = new QVBoxLayout;

    m_topWid = new TopWidget(this);
    m_positionWid = new PositionWidget(this);
    m_bottomWid = new BottomWidget(this);
    m_listWid = new ListWidgets(this);
    m_listWid->setVisible(false);

    BaseWidget* contentWid = new BaseWidget(this);
    contentWid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(contentWid, 2);
    middleLayout->addWidget(m_listWid, 1);

    vmainLayout->addWidget(m_topWid);
    vmainLayout->addLayout(middleLayout);
    vmainLayout->addWidget(m_positionWid);
    vmainLayout->addWidget(m_bottomWid);
    vmainLayout->setMargin(0);
    vmainLayout->setSpacing(0);

    setLayout(vmainLayout);
}

void ControlSurface::initConnetion()
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_hideFurface()));
    connect(m_positionWid, SIGNAL(sliderValueChange(int)), this, SIGNAL(sig_sliderPositionChanged(int)));
}

void ControlSurface::slot_hideFurface()
{
    if (m_bottomWid->isVisible()) {
        m_topWid->setVisible(false);
        m_positionWid->setVisible(false);
        m_bottomWid->setVisible(false);
        m_listWid->setVisible(false);
    }
}

void ControlSurface::slot_showFurface(bool mediaOn)
{
    m_mediaOn = mediaOn;

    if (!m_bottomWid->isVisible()) {
        m_topWid->setVisible(true);
        m_bottomWid->setVisible(true);
    }

    if (mediaOn) {
        restartHideTimer();
        m_positionWid->setVisible(true);
    } else {
        m_timer->stop();
        m_listWid->setVisible(true);
        m_positionWid->setVisible(false);
    }
}

void ControlSurface::removePositionWidget()
{
    m_positionWid->setVisible(false);
}

void ControlSurface::hidePlayList()
{
    m_listWid->setVisible(false);
}

void ControlSurface::showPlayList()
{
    m_listWid->setVisible(true);
}

void ControlSurface::listButtonTrigger()
{
    if (m_listWid->isVisible())
        m_listWid->setVisible(false);
    else
        m_listWid->setVisible(true);
}

void ControlSurface::restartHideTimer()
{
    if (m_mediaOn) {
        m_timer->stop();
        m_timer->start(5000);
    }
}

void ControlSurface::mousePressEvent(QMouseEvent *event)
{
    if (m_positionWid->isVisible() && m_bottomWid->isVisible())
        slot_hideFurface();
    else
        slot_showFurface(m_mediaOn);

    QWidget::mousePressEvent(event);
}

ControlSurface::~ControlSurface()
{
}
