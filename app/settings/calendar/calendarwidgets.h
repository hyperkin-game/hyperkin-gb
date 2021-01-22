#ifndef RIGHTSTACKEDWIDGETS3_H
#define RIGHTSTACKEDWIDGETS3_H

#include <basewidget.h>
#include "widget.h"

class CalendarWidgets : public BaseWidget
{
    Q_OBJECT
public:
    CalendarWidgets(QWidget *parent);
    ~CalendarWidgets();

private:
    Widget *m_calendarWidget;

    void initData();
    void initLayout();
};

#endif // RIGHTSTACKEDWIDGETS3_H
