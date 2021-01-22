#ifndef BOTTOMWIDGETS_H
#define BOTTOMWIDGETS_H

#include "base/basewidget.h"
#include "base/basepushbutton.h"
#include "player/medialist.h"
#include "baseslider.h"
#include "volwidget.h"

#include <QTime>
#include <QLabel>

class BottomWidgets : public BaseWidget
{
    Q_OBJECT
public:
    explicit BottomWidgets(QWidget *parent = 0);
    ~BottomWidgets(){}

    void onPlayerPositionChanged(qint64);
    void onPlayerDurationChanged(qint64);
    void setPauseStyle();
    void setPlayStyle();
    void setOriginState();
    void updateVolumeSliderValue(int);
    void updatePlayModeIcon(PlayMode);

private:
    qint64 m_duration;
    BaseSlider *m_progressSlider;

    QLabel *m_labPosition;
    FlatButton *m_btnNext;
    FlatButton *m_btnPrevious;
    FlatButton *m_btnPlay;
    FlatButton *m_btnPlayMode;
    FlatButton *m_btnRefresh;
    VolWidget *m_volWid;

    void initLayout();
    void initConnection();
    void setPositionLabel(QTime currentTime, QTime totalTime);

signals:
    void nextClick();
    void lastClick();
    void nextLongPressed();
    void lastLongPressed();
    void playPauseClick();
    void progressSliderPositionChanged(int);
    void volumeChanged(int);
    void playModeClick();
    void refreshClick();
};

#endif // BottomWidgets_H
