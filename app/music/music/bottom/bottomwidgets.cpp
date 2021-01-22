#include "bottomwidgets.h"
#include "constant.h"

#include <QHBoxLayout>

#ifdef DEVICE_EVB
int playButton_size = 100;
int bottom_height = 150;
int bottom_spacing = 25;
int layout3_size = 70;
int layout3_temp = 25;
int progress_slider_height = 20;
#else
int playButton_size = 55;
int bottom_height = 80;
int bottom_spacing = 20;
int layout3_size = 45;
int layout3_temp = 10;
int progress_slider_height = 10;
#endif

BottomWidgets::BottomWidgets(QWidget *parent) : BaseWidget(parent)
  , m_duration(-1)
{
    setBackgroundColor(54, 54, 54);

    initLayout();
    initConnection();
}

void BottomWidgets::initLayout()
{
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *mainLayout = new QHBoxLayout;

    /*----------------- position layout -------------------*/
    m_labPosition = new QLabel(this);
    m_labPosition->setAlignment(Qt::AlignCenter);
    BaseWidget::setWidgetFontSize(m_labPosition, font_size_large);

    /* set the whole widget height = this label height + baseSlider height */
    m_labPosition->setFixedHeight(bottom_height);

    /*----------------- play control button ----------------*/
    m_btnNext = new FlatButton(this);
    m_btnPrevious = new FlatButton(this);
    m_btnPlay = new FlatButton(this);

    m_btnNext->setFixedSize(playButton_size, playButton_size);
    m_btnPrevious->setFixedSize(playButton_size, playButton_size);
    m_btnPlay->setFixedSize(playButton_size, playButton_size);

    m_btnNext->setBackgroundImage(":/image/music/btn_next (2).png");
    m_btnPrevious->setBackgroundImage(":/image/music/btn_previous (2).png");
    m_btnPlay->setBackgroundImage(":/image/music/btn_play (2).png");

    QHBoxLayout *playControlLayout = new QHBoxLayout;
    playControlLayout->addWidget(m_btnPrevious);
    playControlLayout->addWidget(m_btnPlay);
    playControlLayout->addWidget(m_btnNext);
    playControlLayout->setMargin(0);
    playControlLayout->setSpacing(bottom_spacing);

    /*----------------- volume、playmode ----------------*/
    m_volWid = new VolWidget(this);

    m_btnPlayMode = new FlatButton(this);
    m_btnPlayMode->setFixedSize(layout3_size, layout3_size);
    m_btnPlayMode->setBackgroundImage(":/image/music/btn_mode_random.png");

    m_btnRefresh = new FlatButton(this);
    m_btnRefresh->setFixedSize(layout3_size, layout3_size);
    m_btnRefresh->setBackgroundImage(":/image/music/btn_refresh.png");
    m_btnRefresh->setVisible(false);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addStretch(0);
    layout3->addWidget(m_btnPlayMode);
    layout3->addWidget(m_volWid);
    layout3->addStretch(0);
    layout3->setMargin(0);
    layout3->setSpacing(bottom_spacing);

    /*-- whole layout contains control layout and progressSlider --*/
    mainLayout->addWidget(m_labPosition, 1);
    mainLayout->addLayout(playControlLayout, 1);
    mainLayout->addLayout(layout3, 1);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    m_progressSlider = new BaseSlider(Qt::Horizontal, this);
    m_progressSlider->setFixedHeight(progress_slider_height);

    layout->addWidget(m_progressSlider);
    layout->addLayout(mainLayout);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setContentsMargins(15,15,15,0);
    setLayout(layout);
}

void BottomWidgets::initConnection()
{
    connect(m_btnPrevious, SIGNAL(longPressedEvent()), this, SIGNAL(lastLongPressed()));
    connect(m_btnNext, SIGNAL(longPressedEvent()), this, SIGNAL(nextLongPressed()));
    connect(m_btnNext, SIGNAL(clicked(bool)), this, SIGNAL(nextClick()));
    connect(m_btnPrevious, SIGNAL(clicked(bool)), this, SIGNAL(lastClick()));
    connect(m_btnPlay, SIGNAL(clicked(bool)), this, SIGNAL(playPauseClick()));
    connect(m_progressSlider, SIGNAL(sig_sliderPositionChanged(int)), this, SIGNAL(progressSliderPositionChanged(int)));
    connect(m_volWid, SIGNAL(sig_valueChanged(int)), this, SIGNAL(volumeChanged(int)));
    connect(m_btnPlayMode, SIGNAL(clicked(bool)), this, SIGNAL(playModeClick()));
    connect(m_btnRefresh, SIGNAL(clicked(bool)), this, SIGNAL(refreshClick()));
}

void BottomWidgets::setPauseStyle()
{
    m_btnPlay->setBackgroundImage(":/image/music/btn_pause (2).png");
}

void BottomWidgets::setPlayStyle()
{
    m_btnPlay->setBackgroundImage(":/image/music/btn_play (2).png");
}

void BottomWidgets::setPositionLabel(QTime currentTime, QTime totalTime)
{
    QString ret;
    ret.append(currentTime.toString("mm:ss")).append("/").append(totalTime.toString("mm:ss"));

    m_labPosition->setText(ret);
}

void BottomWidgets::updateVolumeSliderValue(int value)
{
    m_volWid->updateSlider(value);
}

void BottomWidgets::onPlayerDurationChanged(qint64 duration)
{
    m_duration = duration;
    m_progressSlider->setRange(0, duration);
}

void BottomWidgets::onPlayerPositionChanged(qint64 position)
{
    QTime currentTime((position % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60),
                      (position % (1000 * 60 * 60)) / (1000 * 60),
                      (position % (1000 * 60)) / 1000);
    QTime totalTime((m_duration % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60),
                    (m_duration % (1000 * 60 * 60)) / (1000 * 60),
                    (m_duration % (1000 * 60)) / 1000);
    setPositionLabel(currentTime, totalTime);
    m_progressSlider->setValue(position);
}

void BottomWidgets::updatePlayModeIcon(PlayMode playMode)
{
    switch (playMode) {
    case PlayRandom:
        m_btnPlayMode->setBackgroundImage(":/image/music/btn_mode_random.png");
        break;
    case PlayOneCircle:
        m_btnPlayMode->setBackgroundImage(":/image/music/btn_mode_single.png");
        break;
    case PlayInOrder:
        m_btnPlayMode->setBackgroundImage(":/image/music/btn_mode_list.png");
        break;
    }
}

void BottomWidgets::setOriginState()
{
    m_progressSlider->setValue(0);
    setPositionLabel(QTime(0, 0, 0), QTime(0, 0, 0));
}
