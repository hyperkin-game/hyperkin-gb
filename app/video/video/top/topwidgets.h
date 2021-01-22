#ifndef VIDEOTOPWIDGETS_H
#define VIDEOTOPWIDGETS_H

#include "basepushbutton.h"
#include "basewidget.h"

#include <QLabel>

class TopWidget : public BaseWidget
{
    Q_OBJECT
public:    
    TopWidget(QWidget *parent = 0);
    ~TopWidget();

    void setTitleName(const QString &text);

private:
    FourStateButton *m_btnreturn;
    QLabel *m_titleLabel;

    void initLayout();

signals:
    void returnClick();
};

#endif // VIDEOTOPWIDGETS_H
