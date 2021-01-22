#ifndef VIDEOPOSITIONWIDGET_H
#define VIDEOPOSITIONWIDGET_H

#include "basewidget.h"
#include "positionslider.h"

#include <QLabel>


class PositionWidget : public BaseWidget
{
    Q_OBJECT
public:
    PositionWidget(QWidget *parent = 0);
    ~PositionWidget();

    void onMediaPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void setOriginState();

private:
    PositionSlider *m_slider;
    QLabel *m_currentTime;
    QLabel *m_totalTime;

    void initWidget();

signals:
    void sliderValueChange(int);
};

#endif // VIDEOPOSITIONWIDGET_H
