#ifndef TOPWIDGETS_H
#define TOPWIDGETS_H

#include "base/basewidget.h"

class TopWidgets : public BaseWidget
{
    Q_OBJECT
public:
    TopWidgets(QWidget *parent = 0);
    ~TopWidgets();

private:
    void initLayout();

signals:
    void returnClick();
};

#endif // TOPWIDGETS_H
