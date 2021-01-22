#ifndef SETTINGWIDGETS_H
#define SETTINGWIDGETS_H

#include <basewidget.h>

#include "top/settingtopwidgets.h"
#include "middle/settingmiddlewidgets.h"

class SettingMiddleWidgets;

/**
 * The main layout of settings widgets.It is made up of The following module.
 * include Wifi、Bluetooth、Volumn、calendar etc.
 * This class in charge of the widgets's connecttion(in master control).
 *
 * The layout include top、middle widgets and each has their own
 * layout control and  logic processing.
 */
class SettingWidgets : public BaseWidget
{
public:
    SettingWidgets(QWidget *parent = 0);
    ~SettingWidgets();

public:
    SettingTopWidgets *m_topWid;
    SettingMiddleWidgets *m_middleWid;

private:
    void initLayout();
    void initConnection();
};

#endif // SETTINGWIDGETS_H
