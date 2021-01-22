#ifndef RIGHTSTACKEDWIDGETS4_H
#define RIGHTSTACKEDWIDGETS4_H

#include <basewidget.h>
#include "volumnwidget.h"

class VolumeWidgets:public BaseWidget
{
    Q_OBJECT
public:
    VolumeWidgets(QWidget *parent);
    ~VolumeWidgets();

private:
    VolumnWidget *m_volumnWidget;

    void initData();
    void initLayout();
};

#endif // RIGHTSTACKEDWIDGETS4_H
