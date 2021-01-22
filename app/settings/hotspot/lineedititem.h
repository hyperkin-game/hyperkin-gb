#ifndef LINEEDITITEM_H
#define LINEEDITITEM_H

#include <QLabel>
#include <QLineEdit>

#include "basewidget.h"

class LineEditItem : public BaseWidget
{
public:
    LineEditItem(QWidget *parent = 0);
    void setItem(const QString &name, const QString &value = "");
    void setItemName(const QString &name);
    QString getValue();

private:
    QLabel *nameLabel;
    QLineEdit *valueEdit;

    void initLayout();
};

#endif // LINEEDITITEM_H
