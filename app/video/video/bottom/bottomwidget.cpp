#include "bottomwidget.h"

#include <QHBoxLayout>

#ifdef DEVICE_EVB
int video_bottom_height  = 160;
int video_playButton_size = 100;
int refresh_playmode_size = 70;
int change_surface_size = 55;
int layout_temp = 25;
#else
int video_bottom_height  = 70;
int video_playButton_size = 50;
int refresh_playmode_size = 45;
int change_surface_size = 35;
int layout_temp = 10;
#endif

BottomWidget::BottomWidget(QWidget *parent) : BaseWidget(parent)
{
    m_parent = (ControlSurface*)parent;

    setBackgroundColor(89, 92, 102);
    setFixedHeight(video_bottom_height);
    initLayout();
    initConnection();
}

void BottomWidget::initLayout()
{
    QHBoxLayout *hmainlyout = new QHBoxLayout;

    m_btnPlayPause = new FlatButton(this);
    m_btnNext = new FlatButton(this);
    m_btnLast = new FlatButton(this);
    m_btnOpenFile = new FlatButton(this);
    m_VolWidget = new VolWidget(this);
    m_btnChangeSize = new FlatButton(this);
    m_btnRefresh = new FlatButton(this);
    m_btnPlayMode = new FlatButton(this);
    m_btnPlayList = new FlatButton(this);

    m_btnOpenFile->setVisible(false);
    m_btnRefresh->setVisible(false);
    m_btnChangeSize->setVisible(false);

    m_btnPlayPause->setFixedSize(video_playButton_size, video_playButton_size);
    m_btnNext->setFixedSize(video_playButton_size, video_playButton_size);
    m_btnLast->setFixedSize(video_playButton_size, video_playButton_size);
    m_btnOpenFile->setFixedSize(video_playButton_size, video_playButton_size);
    m_btnChangeSize->setFixedSize(change_surface_size, change_surface_size);
    m_btnRefresh->setFixedSize(refresh_playmode_size, refresh_playmode_size);
    m_btnPlayMode->setFixedSize(refresh_playmode_size, refresh_playmode_size);
    m_btnPlayList->setFixedSize(refresh_playmode_size, refresh_playmode_size);

    m_btnNext->setBackgroundImage(":/image/video/btn_next (2).png");
    m_btnLast->setBackgroundImage(":/image/video/btn_previous (2).png");
    m_btnPlayPause->setBackgroundImage(":/image/video/btn_play (2).png");
    m_btnOpenFile->setBackgroundImage(":/image/video/video_open_file.png");
    m_btnChangeSize->setBackgroundImage(":/image/video/btn_fullscreen.png");
    m_btnRefresh->setBackgroundImage(":/image/video/video_refresh.png");
    m_btnPlayMode->setBackgroundImage(":/image/video/btn_list.png");
    m_btnPlayList->setBackgroundImage(":/image/video/btn_play_list.png");

    QHBoxLayout *hlyout1 = new QHBoxLayout;
    hlyout1->addStretch(0);
    hlyout1->addWidget(m_VolWidget);
    hlyout1->addStretch(0);

    QHBoxLayout *hlyout2 = new QHBoxLayout;
    hlyout2->addWidget(m_btnLast);
    hlyout2->addSpacing(20);
    hlyout2->addWidget(m_btnPlayPause);
    hlyout2->addSpacing(20);
    hlyout2->addWidget(m_btnNext);

    QHBoxLayout *hlyout3 = new QHBoxLayout;
    hlyout3->addStretch(0);
    hlyout3->addWidget(m_btnPlayMode);
    hlyout3->addSpacing(30);
    hlyout3->addWidget(m_btnPlayList);
    hlyout3->addStretch(0);

    hmainlyout->addLayout(hlyout1, 1);
    hmainlyout->addLayout(hlyout2, 1);
    hmainlyout->addLayout(hlyout3, 1);
    hmainlyout->setMargin(0);
    hmainlyout->setSpacing(0);

    setLayout(hmainlyout);
}

void BottomWidget::initConnection()
{
    connect(m_btnOpenFile, SIGNAL(clicked(bool)), this, SIGNAL(openFileClick()));
    connect(m_VolWidget, SIGNAL(sig_valueChanged(int)), this, SIGNAL(volumeValueChanged(int)));
    connect(m_btnPlayPause, SIGNAL(clicked(bool)), this, SIGNAL(playPauseClick()));
    connect(m_btnNext, SIGNAL(clicked(bool)), this, SIGNAL(nextClick()));
    connect(m_btnNext, SIGNAL(longPressedEvent()),this, SIGNAL(nextLongPressed()));
    connect(m_btnLast, SIGNAL(clicked(bool)), this, SIGNAL(lastClick()));
    connect(m_btnLast, SIGNAL(longPressedEvent()), this, SIGNAL(lastLongPressed()));
    connect(m_btnPlayMode, SIGNAL(clicked(bool)), this, SIGNAL(playModeClick()));
    connect(m_btnChangeSize, SIGNAL(clicked(bool)), this, SIGNAL(changeSizeClick()));
    connect(m_btnRefresh, SIGNAL(clicked(bool)), this, SIGNAL(refreshClick()));
    connect(m_btnPlayList, SIGNAL(clicked(bool)), this, SIGNAL(playListClick()));
}

void BottomWidget::setPlayingStyle()
{
    m_btnPlayPause->setBackgroundImage(":/image/video/btn_pause (2).png");

}

void BottomWidget::setPauseStyle()
{
    m_btnPlayPause->setBackgroundImage(":/image/video/btn_play (2).png");
}

void BottomWidget::updatePlayModeIcon(PlayMode playMode)
{
    switch (playMode) {
    case PlayRandom:
        m_btnPlayMode->setBackgroundImage(":/image/video/btn_random.png");
        break;
    case PlayOneCircle:
        m_btnPlayMode->setBackgroundImage(":/image/video/btn_single.png");
        break;
    case PlayInOrder:
        m_btnPlayMode->setBackgroundImage(":/image/video/btn_list.png");
        break;
    default:
        break;
    }
}

void BottomWidget::mousePressEvent(QMouseEvent*)
{
    m_parent->restartHideTimer();
}

void BottomWidget::updateVolumeSliderValue(int value)
{
    m_VolWidget->updateSlider(value);
}

BottomWidget::~BottomWidget()
{
}
