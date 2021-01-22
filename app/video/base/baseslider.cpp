#include "baseslider.h"

BaseSlider::BaseSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
    init();
}

void BaseSlider::init()
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setStyleSheet("QSlider::sub-page:horizontal{background:rgb(26,158,255);}"
                  "QSlider::add-page:horizontal{background:rgb(210,210,210);}"
                  "QSlider::handle:horizontal{background:rgb(255,255,255)}");
}

void BaseSlider::mousePressEvent(QMouseEvent *event)
{
    // position '-1' stands for update state.
    emit sig_sliderPositionChanged(-1);

    QSlider::mousePressEvent(event);
}

void BaseSlider::mouseMoveEvent(QMouseEvent *event)
{
    // position '-1' stands for update state.
    emit sig_sliderPositionChanged(-1);

    QSlider::mouseMoveEvent(event);
}

void BaseSlider::mouseReleaseEvent(QMouseEvent *event)
{
    int dur = maximum() - minimum();
    int pos = minimum() + dur * ((double)event->x() / width());
    if (pos != sliderPosition()) {
        setValue(pos);
        emit sig_sliderPositionChanged(pos);
    }

    QSlider::mouseReleaseEvent(event);
}
