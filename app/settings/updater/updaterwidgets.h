#ifndef RIGHTSTACKEDWIDGETS5_H
#define RIGHTSTACKEDWIDGETS5_H

#include <basewidget.h>
#include "updaterwidget.h"

class UpdaterWidgets : public BaseWidget
{
    Q_OBJECT
public:
    UpdaterWidgets(QWidget *parent);
    ~UpdaterWidgets();

private:
    UpdaterWidget *m_updaterWidget;

    void initData();
    void initLayout();
};

#endif // RIGHTSTACKEDWIDGETS5_H
