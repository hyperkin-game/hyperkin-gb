#include "positionwidget.h"
#include "constant.h"

#include <QHBoxLayout>
#include <QTime>

#ifdef DEVICE_EVB
int video_position_height = 60;
#else
int video_position_height = 35;
#endif

PositionWidget::PositionWidget(QWidget *parent):BaseWidget(parent)
{
    setBackgroundColor(31, 31, 31);

    initWidget();
    setFixedHeight(video_position_height);

    connect(m_slider, SIGNAL(sig_sliderPositionChanged(int)), this, SIGNAL(sliderValueChange(int)));
}

void PositionWidget::initWidget()
{
    QHBoxLayout *layout = new QHBoxLayout;

    m_slider = new PositionSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 0);

    m_currentTime = new QLabel(this);
    m_currentTime->setStyleSheet("color:rgb(150,150,150);");
    m_currentTime->setFixedHeight(video_position_height);
    m_currentTime->setAlignment(Qt::AlignVCenter);

    m_totalTime = new QLabel(this);
    m_totalTime->setStyleSheet("color:rgb(150,150,150);");
    m_totalTime->setFixedHeight(video_position_height);
    m_totalTime->setAlignment(Qt::AlignVCenter);

    layout->addSpacing(10);
    layout->addWidget(m_currentTime);
    layout->addWidget(m_slider);
    layout->addWidget(m_totalTime);
    layout->addSpacing(10);
    layout->setSpacing(10);
    layout->setMargin(0);

    setLayout(layout);
}

void PositionWidget::onDurationChanged(qint64 duration)
{
    m_slider->setRange(0, duration);
    QTime totalTime((duration % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60),
                    (duration % (1000 * 60 * 60)) / (1000 * 60),
                    (duration % (1000 * 60)) / 1000);

    m_totalTime->setText(totalTime.toString("hh:mm:ss"));
}

void PositionWidget::onMediaPositionChanged(qint64 position)
{
    m_slider->setValue(position);
    QTime currentTime((position % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60),
                      (position % (1000 * 60 * 60)) / (1000 * 60),
                      (position % (1000 * 60)) / 1000);

    m_currentTime->setText(currentTime.toString("hh:mm:ss"));
}

void PositionWidget::setOriginState()
{
    m_currentTime->setText("00:00:00");
    m_totalTime->setText("00:00:00");
}

PositionWidget::~PositionWidget()
{
}
