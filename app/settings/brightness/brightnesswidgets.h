#ifndef RIGHTSTACKEDWIDGETS2_H
#define RIGHTSTACKEDWIDGETS2_H

#include <basewidget.h>
#include "brightness.h"

class BrightnessWidgets : public BaseWidget
{
    Q_OBJECT
public:
    BrightnessWidgets(QWidget *parent);
    ~BrightnessWidgets();

private:
    Brightness *m_brightnessWidget;

    void initData();
    void initLayout();
};

#endif // RIGHTSTACKEDWIDGETS2_H
