#ifndef VOLWIDGET_H
#define VOLWIDGET_H

#include "baseslider.h"
#include "basewidget.h"
#include "basepushbutton.h"

class VolWidget : public BaseWidget
{
    Q_OBJECT
public:
    VolWidget(QWidget *parent);
    ~VolWidget(){}

    void updateSlider(int value);

private:
    bool isMute;
    int valueBeforeMute;

    BaseSlider *m_volSlider;
    FlatButton *m_btnIcon;

    void init();
    void updateIconBySliderValue(int);

signals:
    void sig_valueChanged(int);

private slots:
    void slot_onIconClick();
    void slot_onSliderValueChanged(int);
};

#endif // VOLWIDGET_H
