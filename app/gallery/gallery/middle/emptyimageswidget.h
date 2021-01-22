#ifndef EMPTYIMAGESWIDGET_H
#define EMPTYIMAGESWIDGET_H

#include "basewidget.h"

class EmptyImagesWidget : public BaseWidget
{
    Q_OBJECT
public:
    EmptyImagesWidget(QWidget *parent = 0);

private:
    void initLayout();
};

#endif // EMPTYIMAGESWIDGET_H
