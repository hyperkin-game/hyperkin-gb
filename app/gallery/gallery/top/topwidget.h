#ifndef GALLERYTOPWIDGETS_H
#define GALLERYTOPWIDGETS_H

#include "basewidget.h"
#include "basepushbutton.h"

#include <QLabel>

class TopWidget : public BaseWidget
{
    Q_OBJECT
public:
    TopWidget(QWidget *parent);
    ~TopWidget();

    void updateTopTitle(QString title);

private:
    QLabel *m_titleLabel;
    void initLayout();

signals:
    void returnClicked();
};

#endif // GALLERYTOPWIDGETS_H
