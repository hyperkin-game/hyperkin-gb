#ifndef FULLSCREENCONTROLWIDGETS_H
#define FULLSCREENCONTROLWIDGETS_H

#include "basewidget.h"
#include "video/middle/positionwidget.h"
#include "bottom/bottomwidget.h"
#include "top/topwidgets.h"
#include "middle/listwidget.h"

class BottomWidget;
class PositionWidget;

/**
 * Control widget when video surface in fullScreen.
 * It contains a sliderBar and control function of player.
 */
class ControlSurface : public BaseWidget
{
    Q_OBJECT
public:
    ControlSurface(QWidget *parent = 0);
    ~ControlSurface();

    BottomWidget* getBottomWidget()
    {
        return m_bottomWid;
    }

    PositionWidget* getPositionWidget()
    {
        return m_positionWid;
    }

    TopWidget* getTopWidget()
    {
        return m_topWid;
    }

    ListWidgets* getListWidget()
    {
        return m_listWid;
    }

    void removePositionWidget();
    void listButtonTrigger();
    void restartHideTimer();

private:
    // hide control Widget if 5 seconds pass and no more action.
    bool m_mediaOn;
    QTimer *m_timer;

    PositionWidget *m_positionWid;
    BottomWidget *m_bottomWid;
    TopWidget *m_topWid;
    ListWidgets *m_listWid;

    void initLayout();
    void initConnetion();

protected:
    void mousePressEvent(QMouseEvent*);

signals:
    void sig_sliderPositionChanged(int);

public slots:
    void slot_hideFurface();
    void slot_showFurface(bool mediaOn = false);
    void hidePlayList();
    void showPlayList();
};

#endif // FULLSCREENCONTROLWIDGETS_H
