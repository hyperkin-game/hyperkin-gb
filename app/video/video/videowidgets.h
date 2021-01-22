#ifndef VIDEOWIDGETS_H
#define VIDEOWIDGETS_H

#include <QMediaPlayer>
#include <QThread>

#include "basewidget.h"
#include "middle/contentwidget.h"
#include "controlsurface.h"

enum CheckState
{
    RESOLUTION_SUITABLE,
    RESOLUTION_UNSUITABLE,
    RESOLUTION_UNCHECKED
};

class MediaLoadThread;
class CheckResolutionThread;

/**
 * The whole application contains here. and the layout of VideoWidgets is
 * a StackedLayout which made up of a control surface and a video surface.
 *
 * Note: this widget do master control of all widget in application.
 */
class VideoWidgets : public BaseWidget
{
    Q_OBJECT
public:
    VideoWidgets(QWidget *parent = 0);
    ~VideoWidgets();

    void savaSetting();
    void updateVolume(bool volumeAdd);
    void setPlayerPause();
    void updateUiByRes(QFileInfoList);
    void showControlView();
    QMap<QString, bool>* getResolutionMap()
    {
        return &m_resolutionMap;
    }

private:
    QMediaPlayer *m_player;
    MediaLoadThread *m_mediaLoadThread;
    CheckResolutionThread *m_checkThread;
    QMap<QString, bool> m_resolutionMap;

    ContentWidget *m_contentWid;
    ControlSurface *m_controlSurface;

    bool m_audioSetup;

    // save normal size of each moudle.
    int top_normal_height;
    int bottom_normal_height;

    void initLayout();
    void initPlayerAndConnection();
    void readSetting();
    void setOriginState();
    CheckState resolutionCheck(const QString &path);
    void showUnsupportDialog();
    void stopAudioService();

signals:
    void resolutionCheckResultCome(QString, bool);

private slots:
    void slot_onMediaStateChanged(QMediaPlayer::MediaStatus);
    void slot_onPlayerStateChanged(QMediaPlayer::State);
    void slot_onMediaPositionChanged(qint64);
    void slot_onDurationChanged(qint64);
    void slot_onSliderPositionChanged(int);
    void slot_onErrorOn(QMediaPlayer::Error);
    void slot_onCurrentMediaChanged(QMediaContent);

    void slot_onLocalListItemClick(int,int);
    void slot_setPlayPause();
    void slot_nextVideo(bool hideFurfaceAfterSet = false);
    void slot_lastVideo();
    void slot_volumeChanged(int);
    void slot_changePlayMode();
    void slot_fastForward();
    void slot_fastBackward();
    void slot_onListButtonTrigger();
    void slot_tableLongPressed(int);
    void slot_checkResultAvailable(QString, bool);
    void slot_updateResolutionMap(QString, bool);

public slots:
    void slot_exit();
};

class MediaLoadThread : public QThread
{
public:
    MediaLoadThread(QObject *parent, QMediaPlayer *player);
    ~MediaLoadThread();

    void setOnPlayUrl(QUrl url);

protected:
    void run();

private:
    QMediaPlayer *m_player;
    QUrl m_loadUrl;
};

class CheckResolutionThread : public QThread
{
    Q_OBJECT
public:
    CheckResolutionThread(QObject *parent);
    ~CheckResolutionThread();

    void setCheckPath(const QString &path);

protected:
    void run();

private:
    QString m_path;

signals:
    void resolutionCheckResultCome(QString, bool);
};

#endif // VIDEOWIDGETS_H
