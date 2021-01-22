#ifndef VIDEOSLIDER_H
#define VIDEOSLIDER_H

#include "baseslider.h"

class PositionSlider : public BaseSlider
{
    Q_OBJECT
public:
    PositionSlider(Qt::Orientation orientation, QWidget *parent);
    ~PositionSlider(){}
};

#endif // VIDEOSLIDER_H
