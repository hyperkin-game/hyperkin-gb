#ifndef IMAGECONTROLER_H
#define IMAGECONTROLER_H

#include "basewidget.h"
#include "basepushbutton.h"

class ImageControler : public BaseWidget
{
public:
    ImageControler(QWidget *parent = 0);

    FlatButton *m_btnLast;
    FlatButton *m_btnZoomOut;
    FlatButton *m_btnZoomIn;
    FlatButton *m_btnRotate;
    FlatButton *m_btnDetail;
    FlatButton *m_btnDelete;
    FlatButton *m_btnNext;

private:
    void initLayout();
};

#endif // IMAGECONTROLER_H
