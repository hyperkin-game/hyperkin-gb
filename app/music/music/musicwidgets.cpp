#include "musicwidgets.h"
#include "constant.h"

#include <QVBoxLayout>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>
#include <QDir>

MusicWidgets::MusicWidgets(QWidget *parent) : BaseWidget(parent)
{
    setTextColorWhite();
    initData();
    initLayout();
    initPlayerAndConnection();

    readSetting();
    setOriginState();
}

void MusicWidgets::readSetting()
{
    // read configuration information first when start application
    // it mainly contain volume and play mode.
    QSettings setting("config.ini", QSettings::IniFormat, 0);
    setting.beginGroup("musicConfig");

    int playModeIndex = 0;
    playModeIndex = setting.value("playmode").toInt();

    MediaList *playList = m_middlewid->getListWidget()->getMediaList();
    playList->setPlayMode(PlayMode(playModeIndex));
    m_bottomwid->updatePlayModeIcon(PlayMode(playModeIndex));
    m_player->currentPlayModeChanged(PlayMode(playModeIndex));

    // set volume which saved in configration file.
    int volumeInt = 0;
    volumeInt = setting.value("volume").toInt();

    m_player->setVolume(volumeInt);
    m_bottomwid->updateVolumeSliderValue(volumeInt);

    setting.endGroup();
}

void MusicWidgets::initData()
{
    m_player = new MusicPlayer(this);
    m_player->connectToService();
}

void MusicWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;

    m_topwid = new TopWidgets(this);
    m_bottomwid = new BottomWidgets(this);
    m_middlewid = new MiddleWidgets(this);

    vmainlyout->addWidget(m_topwid);
    vmainlyout->addWidget(m_middlewid);
    vmainlyout->addWidget(m_bottomwid);
    vmainlyout->setSpacing(0);
    vmainlyout->setMargin(0);

    setLayout(vmainlyout);
}

void MusicWidgets::initPlayerAndConnection()
{
    connect(m_player, SIGNAL(mediaStatusChanged(MusicPlayer::MediaStatus)),
            this, SLOT(slot_onMediaStatusChanged(MusicPlayer::MediaStatus)));
    connect(m_player, SIGNAL(error(MusicPlayer::Error)),
            this, SLOT(slot_onErrorOn(MusicPlayer::Error)));
    connect(m_player, SIGNAL(stateChanged(MusicPlayer::State)),
            this, SLOT(slot_onStateChanged(MusicPlayer::State)));
    connect(m_player, SIGNAL(metaDataAvailable()), this, SLOT(slot_onMetaDataAvailable()));
    connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(slot_onPositonChanged(qint64)));
    connect(m_player, SIGNAL(durationChanged(qint64)), this, SLOT(slot_onDuratuonChanged(qint64)));

    connect(m_topwid, SIGNAL(returnClick()), this, SLOT(slot_exit()));

    connect(m_bottomwid, SIGNAL(nextClick()), this, SLOT(slot_nextSong()));
    connect(m_bottomwid, SIGNAL(lastClick()), this, SLOT(slot_preSong()));
    connect(m_bottomwid, SIGNAL(nextLongPressed()), this, SLOT(slot_fastForward()));
    connect(m_bottomwid, SIGNAL(lastLongPressed()), this, SLOT(slot_fastBackward()));
    connect(m_bottomwid, SIGNAL(playPauseClick()), this, SLOT(slot_playOrPause()));
    connect(m_bottomwid, SIGNAL(progressSliderPositionChanged(int)),
            this, SLOT(slot_changePlayerProgress(int)));
    connect(m_bottomwid, SIGNAL(volumeChanged(int)), this, SLOT(slot_volumeChanged(int)));
    connect(m_bottomwid, SIGNAL(playModeClick()), this, SLOT(slot_changePlayMode()));
    connect(m_bottomwid, SIGNAL(refreshClick()), this, SLOT(slot_refreshMediaResource()));

    connect(m_middlewid->getListWidget(), SIGNAL(tableClick(int,int)),
            this, SLOT(slot_onTableItemClicked(int,int)));
    connect(m_middlewid->getListWidget(), SIGNAL(tableLongPressed(int)),
            this, SLOT(slot_deleteTableItem(int)));
}

void MusicWidgets::slot_volumeChanged(int value)
{
    m_player->setVolume(value);
}

void MusicWidgets::slot_onErrorOn(MusicPlayer::Error)
{
    m_player->setMedia(NULL);
    setOriginState();

    QMessageBox *messageBox = new QMessageBox(QMessageBox::Critical, tr("format problem"),
                                              tr("Audio format error"), QMessageBox::Yes, mainWindow);
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    QTimer::singleShot(2500, messageBox, SLOT(close()));
    messageBox->exec();
}

void MusicWidgets::slot_onMediaStatusChanged(MusicPlayer::MediaStatus status)
{
    switch (status) {
    case MusicPlayer::EndOfMedia:
        slot_nextSong();
        break;
    default:
        break;
    }
}

void MusicWidgets::setOriginState()
{
    m_bottomwid->setOriginState();
    m_middlewid->getListWidget()->setOriginState();
    m_middlewid->getLyricWidget()->setOriginState();
}

void MusicWidgets::slot_onStateChanged(MusicPlayer::State state)
{
    if (state == MusicPlayer::PlayingState)
        m_bottomwid->setPauseStyle();
    else
        m_bottomwid->setPlayStyle();
}

void MusicWidgets::slot_onMetaDataAvailable()
{
    QString mediaName = m_player->currentMedia();

    m_middlewid->getLyricWidget()->currentMediaChanged(m_player->getMediaTitle(), mediaName);
    m_middlewid->getListWidget()->setPlayingMediaContent(mediaName);
}

void MusicWidgets::slot_onPositonChanged(qint64 position)
{
    m_bottomwid->onPlayerPositionChanged(position);
    m_middlewid->getLyricWidget()->onCurrentPositionChanged(position);
}

void MusicWidgets::slot_onDuratuonChanged(qint64 duration)
{
    m_bottomwid->onPlayerDurationChanged(duration);
}

void MusicWidgets::slot_changePlayerProgress(int position)
{
    if (position >= 0)
        m_player->setPosition(position);
}

void MusicWidgets::slot_changePlayMode()
{
    MediaList *playList = m_middlewid->getListWidget()->getMediaList();
    playList->changePlayMode();

    m_bottomwid->updatePlayModeIcon(playList->getCurrentPlayMode());
    m_player->currentPlayModeChanged(playList->getCurrentPlayMode());
}

void MusicWidgets::slot_refreshMediaResource()
{  
    mainWindow->slot_updateMedia();
}

void MusicWidgets::slot_onTableItemClicked(int row, int)
{
    MediaList *playlist = m_middlewid->getListWidget()->getMediaList();
    QString filePath = playlist->getPathAt(row);

    if (m_player->isAvailable()) {
        m_player->setMedia(filePath);
        m_player->play();
    }
}

void MusicWidgets::slot_deleteTableItem(int row)
{
    QMessageBox box(QMessageBox::Warning, tr("question"), tr("sure you want to remove the record ?"));
    box.setStandardButtons (QMessageBox::Yes | QMessageBox::Cancel);
    if (box.exec() == QMessageBox::Yes) {
        MediaList *playlist = m_middlewid->getListWidget()->getMediaList();
        QFile file(playlist->getPathAt(row));
        if (file.exists())
            file.remove();

        m_middlewid->getListWidget()->deleteItem(row);
    }
}

void MusicWidgets::slot_fastForward()
{
    if (m_player->state() == MusicPlayer::PlayingState
            || m_player->state() == MusicPlayer::PausedState) {
        m_player->setPosition(m_player->position() + 5000);
    }
}

void MusicWidgets::slot_fastBackward()
{
    if (m_player->state() == MusicPlayer::PlayingState
            || m_player->state() == MusicPlayer::PausedState) {
        m_player->setPosition(m_player->position() - 5000);
    }
}

void MusicWidgets::slot_nextSong()
{
    MediaList *playlist = m_middlewid->getListWidget()->getMediaList();
    if (m_player->isAvailable()) {
        m_player->setMedia(playlist->getNextSongPath());
        m_player->play();
    }

    if (playlist->getPathList().size() == 0) {
        setOriginState();
    }
}

void MusicWidgets::slot_preSong()
{
    m_player->stop();
    MediaList *playlist = m_middlewid->getListWidget()->getMediaList();
    if (m_player->isAvailable()) {
        m_player->setMedia(playlist->getPreSongPath());
        m_player->play();
    }

    if (playlist->getPathList().size() == 0) {
        setOriginState();
    }
}

void MusicWidgets::slot_playOrPause()
{
    if (m_player->state() == MusicPlayer::PlayingState) {
        m_player->pause();
    } else {
        if (m_player->isAudioAvailable() == true)
            m_player->play();
    }
}

void MusicWidgets::updateUiByRes(QFileInfoList fileInfoList)
{
    m_middlewid->getListWidget()->updateLocalList(fileInfoList);

    if (m_player->currentMedia() != "")
        slot_onMetaDataAvailable();
}

void MusicWidgets::savaSetting()
{
    // saved play mode and volume when exit application.
    QSettings setting("config.ini", QSettings::IniFormat, 0);
    setting.beginGroup("musicConfig");
    setting.setValue("playmode", (int)m_middlewid->getListWidget()->getMediaList()->getPlayMode());
    setting.setValue("volume", m_player->volume());
    setting.endGroup();
}

void MusicWidgets::slot_exit()
{
    savaSetting();
    m_player->clientExit();
    mainWindow->exitApplication();
}

void MusicWidgets::updateVolume(bool volumeAdd)
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
    m_bottomwid->updateVolumeSliderValue(m_player->volume());
}

MusicWidgets::~MusicWidgets()
{
}
