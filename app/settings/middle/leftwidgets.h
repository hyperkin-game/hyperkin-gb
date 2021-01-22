#ifndef LEFTWIDGETS_H
#define LEFTWIDGETS_H

#include "basewidget.h"
#include "funtiontablewidget.h"

class LeftWidgets:public BaseWidget
{
public:
    LeftWidgets(QWidget *parent=0);
    ~LeftWidgets();

    Funtiontablewidget* getList(){return m_funtionlist;}

private:
    void initLayout();

    Funtiontablewidget *m_funtionlist;
};

#endif // LEFTWIDGETS_H
