#ifndef VIDEOBOTTOMWIDGETS_H
#define VIDEOBOTTOMWIDGETS_H

#include "basewidget.h"
#include "basepushbutton.h"
#include "volwidget.h"
#include "controlsurface.h"
#include "player/medialist.h"

class ControlSurface;

class BottomWidget : public BaseWidget
{
    Q_OBJECT
public:
    BottomWidget(QWidget *parent = 0);
    ~BottomWidget();

    void setPlayingStyle();
    void setPauseStyle();
    void updatePlayModeIcon(PlayMode playMode);
    void updateVolumeSliderValue(int value);

private:
    ControlSurface *m_parent;

    FlatButton *m_btnOpenFile;
    FlatButton *m_btnPlayPause;
    FlatButton *m_btnNext;
    FlatButton *m_btnLast;
    VolWidget *m_VolWidget;
    FlatButton *m_btnChangeSize;
    FlatButton *m_btnRefresh;
    FlatButton *m_btnPlayMode;
    FlatButton *m_btnPlayList;

    void initLayout();
    void initConnection();

protected:
    void mousePressEvent(QMouseEvent *);

signals:
    void openFileClick();
    void volumeValueChanged(int);
    void nextClick();
    void lastClick();
    void nextLongPressed();
    void lastLongPressed();
    void playPauseClick();
    void playModeClick();
    void refreshClick();
    void changeSizeClick();
    void playListClick();
};

#endif // VIDEOBOTTOMWIDGETS_H
