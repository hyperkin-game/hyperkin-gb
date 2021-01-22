#include "videowidgets.h"
#include "player/videoinfoutil.h"
#include "player/medialist.h"
#include "constant.h"
/* audio service included */
#include "musicplayer.h"

#include <QHBoxLayout>
#include <QStackedLayout>
#include <QSettings>
#include <QMessageBox>
#include <QDir>

VideoWidgets::VideoWidgets(QWidget *parent) : BaseWidget(parent)
  , m_mediaLoadThread(0)
  , m_checkThread(0)
  , m_audioSetup(false)
{
    setStyleSheet("QLabel{color:white;}");

    initLayout();
    initPlayerAndConnection();
    readSetting();
    setOriginState();
}

void VideoWidgets::readSetting()
{
    // read configuration information first when start application
    // it mainly contain volume and play mode.
    QSettings setting("config.ini", QSettings::IniFormat, 0);
    setting.beginGroup("videoConfig");

    int playModeIndex = 0;
    playModeIndex = setting.value("playmode").toInt();

    MediaList *playList = m_controlSurface->getListWidget()->getMediaList();
    playList->setPlayMode((PlayMode)playModeIndex);
    m_controlSurface->getBottomWidget()->updatePlayModeIcon((PlayMode)playModeIndex);

    // set volume which saved in configration file.
    int volumeInt = 0;
    volumeInt = setting.value("volume").toInt();

    m_player->setVolume(volumeInt);
    m_controlSurface->getBottomWidget()->updateVolumeSliderValue(volumeInt);

    setting.endGroup();
}

void VideoWidgets::initLayout()
{
    QStackedLayout *layout = new QStackedLayout;

    m_controlSurface = new ControlSurface(this);

    m_contentWid = new ContentWidget(this);
    m_contentWid->setWindowFlags(Qt::FramelessWindowHint);
    m_contentWid->setAttribute(Qt::WA_TranslucentBackground, true);

    layout->addWidget(m_controlSurface);
    layout->addWidget(m_contentWid);
    layout->setStackingMode(QStackedLayout::StackAll);

    setLayout(layout);
}

void VideoWidgets::initPlayerAndConnection()
{
    m_player = m_contentWid->getMediaPlayerFormQml();
    m_mediaLoadThread = new MediaLoadThread(this, m_player);
    m_checkThread = new CheckResolutionThread(this);

    connect(this, SIGNAL(resolutionCheckResultCome(QString,bool)),
            this, SLOT(slot_updateResolutionMap(QString,bool)));
    connect(m_checkThread, SIGNAL(resolutionCheckResultCome(QString,bool)), this,
            SLOT(slot_checkResultAvailable(QString,bool)));

    connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slot_onMediaStateChanged(QMediaPlayer::MediaStatus)));
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(slot_onPlayerStateChanged(QMediaPlayer::State)));
    connect(m_player, SIGNAL(currentMediaChanged(QMediaContent)),
            this, SLOT(slot_onCurrentMediaChanged(QMediaContent)));
    connect(m_player, SIGNAL(positionChanged(qint64)),
            this, SLOT(slot_onMediaPositionChanged(qint64)));
    connect(m_player, SIGNAL(durationChanged(qint64)),
            this, SLOT(slot_onDurationChanged(qint64)));
    connect(m_player, SIGNAL(error(QMediaPlayer::Error)),
            this, SLOT(slot_onErrorOn(QMediaPlayer::Error)));

    connect(m_controlSurface->getTopWidget(), SIGNAL(returnClick()),
            this, SLOT(slot_exit()));
    connect(m_controlSurface->getListWidget(), SIGNAL(sig_localTableItemClick(int,int)),
            this, SLOT(slot_onLocalListItemClick(int,int)));
    connect(m_controlSurface->getListWidget(), SIGNAL(tableLongPressed(int)),
            this, SLOT(slot_tableLongPressed(int)));
    connect(m_controlSurface, SIGNAL(sig_sliderPositionChanged(int)),
            this, SLOT(slot_onSliderPositionChanged(int)));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(playPauseClick()),
            this, SLOT(slot_setPlayPause()));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(nextClick()),
            this, SLOT(slot_nextVideo()));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(lastClick()),
            this, SLOT(slot_lastVideo()));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(nextLongPressed()),
            this, SLOT(slot_fastForward()));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(lastLongPressed()),
            this, SLOT(slot_fastBackward()));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(volumeValueChanged(int)),
            this, SLOT(slot_volumeChanged(int)));
    connect(m_controlSurface->getPositionWidget(), SIGNAL(sliderValueChange(int)),
            this, SLOT(slot_onSliderPositionChanged(int)));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(playModeClick()),
            this, SLOT(slot_changePlayMode()));
    connect(m_controlSurface->getBottomWidget(), SIGNAL(playListClick()),
            this, SLOT(slot_onListButtonTrigger()));
}

void VideoWidgets::setOriginState()
{
    m_controlSurface->slot_showFurface(false);
    m_controlSurface->getTopWidget()->setTitleName(tr("videoPlayer"));
    m_controlSurface->getListWidget()->setOriginState();
    m_controlSurface->getPositionWidget()->setOriginState();
}

void VideoWidgets::showControlView()
{
    m_controlSurface->slot_showFurface(m_player->state() != QMediaPlayer::StoppedState);
}

void VideoWidgets::setPlayerPause()
{
    m_player->pause();
}

void VideoWidgets::updateUiByRes(QFileInfoList fileInfoList)
{
    m_controlSurface->getListWidget()->updateResUi(fileInfoList);

    if (m_player->currentMedia().canonicalUrl().toString() != "")
        slot_onCurrentMediaChanged(m_player->currentMedia());
}

void VideoWidgets::showUnsupportDialog()
{
    QMessageBox box(QMessageBox::Critical, tr("Video Format Error"),
                    tr("video resolution not support."), QMessageBox::Yes);
    setOriginState();
    m_player->setMedia(NULL);
    QTimer::singleShot(2500, &box, SLOT(close()));
    box.exec();
}

void VideoWidgets::slot_updateResolutionMap(QString path, bool isSuitable)
{
    m_resolutionMap.insert(path, isSuitable);
}

void VideoWidgets::slot_checkResultAvailable(QString path, bool isSuitable)
{
    slot_updateResolutionMap(path, isSuitable);
    if (!isSuitable)
        showUnsupportDialog();
    else if (!m_mediaLoadThread->isRunning())
        m_mediaLoadThread->start();
}

void VideoWidgets::slot_onErrorOn(QMediaPlayer::Error)
{
    setOriginState();

    QMessageBox *messageBox = new QMessageBox(QMessageBox::Critical,
                                              tr("Video Error"),
                                              tr("It has encountered an error."),
                                              QMessageBox::Yes, mainWindow);
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    QTimer::singleShot(2500, messageBox, SLOT(close()));
    messageBox->exec();
}

void VideoWidgets::slot_onMediaStateChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::EndOfMedia:
        slot_nextVideo(true);
        break;
    default:
        break;
    }
}

void VideoWidgets::slot_onPlayerStateChanged(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        stopAudioService();
        m_controlSurface->getBottomWidget()->setPlayingStyle();
        break;
    case QMediaPlayer::PausedState:
        m_controlSurface->getBottomWidget()->setPauseStyle();
        break;
    default:
        m_controlSurface->getBottomWidget()->setPauseStyle();
        break;
    }
}

void VideoWidgets::stopAudioService()
{
    if (!m_audioSetup) {
        MusicPlayer audioService;
        audioService.connectToService();
        if (audioService.state() == MusicPlayer::PlayingState) {
            audioService.pause();
        }
        audioService.clientExit();
        m_audioSetup = true;
    }
}

void VideoWidgets::slot_onCurrentMediaChanged(QMediaContent content)
{
    if (content.canonicalUrl().toString() != "") {
        m_controlSurface->getListWidget()->updatePlayingItemStyle(content);
        m_controlSurface->getTopWidget()->setTitleName(content.canonicalUrl().fileName());
    }
}

void VideoWidgets::slot_onLocalListItemClick(int row, int)
{
    if (m_mediaLoadThread->isRunning())
        return;

    QUrl url= m_controlSurface->getListWidget()->getMediaList()->getUrlAt(row);
    if (m_player->isAvailable()) {
        m_controlSurface->hidePlayList();
        m_controlSurface->slot_showFurface(true);
#ifdef DEVICE_EVB
        m_mediaLoadThread->setOnPlayUrl(url);
        m_mediaLoadThread->start();
#else
        CheckState state = resolutionCheck(url.path());
        switch (state) {
        case RESOLUTION_SUITABLE:
            m_mediaLoadThread->setOnPlayUrl(url);
            m_mediaLoadThread->start();
            break;
        case RESOLUTION_UNSUITABLE:
            showUnsupportDialog();
            break;
        case RESOLUTION_UNCHECKED:
            m_mediaLoadThread->setOnPlayUrl(url);
            break;
        default:
            break;
        }
#endif
    }
}

void VideoWidgets::slot_nextVideo(bool hideFurfaceAfterSet)
{
    if (m_mediaLoadThread->isRunning())
        return;

    QUrl url = m_controlSurface->getListWidget()->getMediaList()->getNextVideoUrl();
    if (m_player->isAvailable()) {
        m_controlSurface->hidePlayList();
        if (!hideFurfaceAfterSet)
            m_controlSurface->slot_showFurface(true);
#ifdef DEVICE_EVB
        m_mediaLoadThread->setOnPlayUrl(url);
        m_mediaLoadThread->start();
#else
        CheckState state = resolutionCheck(url.path());
        switch (state) {
        case RESOLUTION_SUITABLE:
            m_mediaLoadThread->setOnPlayUrl(url);
            m_mediaLoadThread->start();
            break;
        case RESOLUTION_UNSUITABLE:
            showUnsupportDialog();
            break;
        case RESOLUTION_UNCHECKED:
            m_mediaLoadThread->setOnPlayUrl(url);
            break;
        default:
            break;
        }
#endif
    }
}

void VideoWidgets::slot_lastVideo()
{
    if (m_mediaLoadThread->isRunning())
        return;

    QUrl url = m_controlSurface->getListWidget()->getMediaList()->getPreVideoUrl();
    if (m_player->isAvailable()) {
        m_controlSurface->hidePlayList();
        m_controlSurface->slot_showFurface(true);
#ifdef DEVICE_EVB
        m_mediaLoadThread->setOnPlayUrl(url);
        m_mediaLoadThread->start();
#else
        CheckState state = resolutionCheck(url.path());
        switch (state) {
        case RESOLUTION_SUITABLE:
            m_mediaLoadThread->setOnPlayUrl(url);
            m_mediaLoadThread->start();
            break;
        case RESOLUTION_UNSUITABLE:
            showUnsupportDialog();
            break;
        case RESOLUTION_UNCHECKED:
            m_mediaLoadThread->setOnPlayUrl(url);
            break;
        default:
            break;
        }
#endif
    }
}

CheckState VideoWidgets::resolutionCheck(const QString &path)
{
    if (m_resolutionMap.contains(path)) {
        return m_resolutionMap.value(path) ? RESOLUTION_SUITABLE : RESOLUTION_UNSUITABLE;
    } else if (!m_checkThread->isRunning()) {
        m_checkThread->setCheckPath(path);
        m_checkThread->start();
    }

    return RESOLUTION_UNCHECKED;
}

void VideoWidgets::slot_fastForward()
{
    if (m_player->state() == QMediaPlayer::PlayingState
            || m_player->state() == QMediaPlayer::PausedState)
        m_player->setPosition(m_player->position() + 5000);

    m_controlSurface->restartHideTimer();
}

void VideoWidgets::slot_fastBackward()
{
    if (m_player->state() == QMediaPlayer::PlayingState
            || m_player->state() == QMediaPlayer::PausedState)
        m_player->setPosition(m_player->position() - 5000);

    m_controlSurface->restartHideTimer();
}

void VideoWidgets::slot_tableLongPressed(int row)
{
    QMessageBox box(QMessageBox::Warning, tr("question"), tr("sure you want to remove the record ?"));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    if (box.exec() == QMessageBox::Yes) {
        m_controlSurface->getListWidget()->deleteItem(row);
        m_controlSurface->getListWidget()->updatePlayingItemStyle(m_player->currentMedia());
    }
}

void VideoWidgets::slot_setPlayPause()
{
    if (m_player->state() == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        if (m_player->isAudioAvailable() == true)
            m_player->play();
    }

    m_controlSurface->restartHideTimer();
}

void VideoWidgets::slot_onListButtonTrigger()
{
    m_controlSurface->listButtonTrigger();
    m_controlSurface->restartHideTimer();
}

void VideoWidgets::slot_changePlayMode()
{
    MediaList *playList = m_controlSurface->getListWidget()->getMediaList();
    playList->changePlayMode();
    m_controlSurface->getBottomWidget()->updatePlayModeIcon(playList->getPlayMode());

    m_controlSurface->restartHideTimer();
}

void VideoWidgets::slot_onDurationChanged(qint64 duration)
{
    m_controlSurface->getPositionWidget()->onDurationChanged(duration);
}

void VideoWidgets::slot_onMediaPositionChanged(qint64 position)
{
    m_controlSurface->getPositionWidget()->onMediaPositionChanged(position);
}

void VideoWidgets::slot_onSliderPositionChanged(int position)
{
    if (position >= 0) {
        m_player->setPosition(position);
        m_controlSurface->restartHideTimer();
    }
}

void VideoWidgets::slot_volumeChanged(int value)
{
    m_player->setVolume(value);
    m_controlSurface->getBottomWidget()->updateVolumeSliderValue(m_player->volume());
    m_controlSurface->restartHideTimer();
}

void VideoWidgets::slot_exit()
{
    savaSetting();
    mainWindow->exitApplication();
}

void VideoWidgets::savaSetting()
{
    // saved play mode when exit application.
    QSettings setting("config.ini", QSettings::IniFormat, 0);
    setting.beginGroup("videoConfig");
    setting.setValue("playmode", (int)m_controlSurface->getListWidget()->getMediaList()->getPlayMode());
    setting.setValue("volume", m_player->volume());
    setting.endGroup();
}

void VideoWidgets::updateVolume(bool volumeAdd)
{
    int currenVolume = m_player->volume();
    if (volumeAdd) {
        if (currenVolume < 95)
            m_player->setVolume(currenVolume + 5);
        else
            m_player->setVolume(100);
    } else {
        if (currenVolume > 5)
            m_player->setVolume(currenVolume - 5);
        else
            m_player->setVolume(0);
    }

    m_controlSurface->getBottomWidget()->updateVolumeSliderValue(m_player->volume());
}

VideoWidgets::~VideoWidgets()
{
}

MediaLoadThread::MediaLoadThread(QObject *parent, QMediaPlayer *player)
    : QThread(parent)
{
    this->m_player = player;
}

MediaLoadThread::~MediaLoadThread()
{
    requestInterruption();
    quit();
    wait();
}

void MediaLoadThread::setOnPlayUrl(QUrl url)
{
    this->m_loadUrl = url;
}

void MediaLoadThread::run()
{
    m_player->setMedia(m_loadUrl);
    m_player->play();
    QThread::msleep(500);
}

CheckResolutionThread::CheckResolutionThread(QObject *parent) : QThread(parent)
{
}

CheckResolutionThread::~CheckResolutionThread()
{
    requestInterruption();
    quit();
    wait();
}

void CheckResolutionThread::setCheckPath(const QString &path)
{
    m_path = path;
}

void CheckResolutionThread::run()
{
    bool isSuitable = VideoInfoUtil::isVideoSolutionSuitable(m_path);
    emit resolutionCheckResultCome(m_path, isSuitable);
}
