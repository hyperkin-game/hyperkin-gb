#include "lineedititem.h"

#include <QHBoxLayout>

#include "constant.h"

LineEditItem::LineEditItem(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(95, 99, 104);
    initLayout();
}

void LineEditItem::initLayout()
{
    QHBoxLayout *layout = new QHBoxLayout;

    nameLabel = new QLabel(this);
    nameLabel->setStyleSheet("background-color:rgb(69,75,83);");
    nameLabel->setContentsMargins(0, 10, 10, 10);

#ifdef DEVICE_EVB
    nameLabel->setFixedWidth(200);
#else
    nameLabel->setFixedWidth(100);
#endif

    nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    valueEdit = new QLineEdit(this);
    valueEdit->setStyleSheet("background:transparent;color:white");

    layout->addWidget(nameLabel);
    layout->addWidget(valueEdit);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);
}

void LineEditItem::setItem(const QString &name, const QString &value)
{
    nameLabel->setText(name);
    valueEdit->setText(value);
}

void LineEditItem::setItemName(const QString &name)
{
    nameLabel->setText(name);
}

QString LineEditItem::getValue()
{
    return valueEdit->text();
}
