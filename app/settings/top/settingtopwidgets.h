#ifndef SETTINGTOPWIDGETS_H
#define SETTINGTOPWIDGETS_H

#include "basewidget.h"
#include "basepushbutton.h"

#include <QLabel>

class SettingTopWidgets : public BaseWidget
{
    Q_OBJECT
public:
    SettingTopWidgets(QWidget *parent = 0);
    ~SettingTopWidgets();

private:
    void initLayout();
    void initConnection();

    FourStateButton *m_btnReturn;
    FlatButton *m_btnIcon;
    QLabel *titleText;

signals:
    void returnClicked();

private slots:
    void retranslateUi();
};

#endif // SETTINGTOPWIDGETS_H
