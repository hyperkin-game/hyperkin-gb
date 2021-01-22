#include "topwidgets.h"
#include "constant.h"
#include "base/basepushbutton.h"

#include <QHBoxLayout>
#include <QLabel>

TopWidgets::TopWidgets(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(231, 155, 155);

    initLayout();
}

void TopWidgets::initLayout()
{
    QHBoxLayout *hmainyout = new QHBoxLayout;

    FourStateButton *btnReturn = new FourStateButton(return_resource_str, this);
    btnReturn->setFixedSize(return_icon_width, return_icon_height);

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(btnReturn);
    layout1->addStretch(0);

    QLabel *titleLabel=new QLabel(tr("music player"), this);
    BaseWidget::setWidgetFontSize(titleLabel, font_size_big);
    titleLabel->setAlignment(Qt::AlignCenter);

    hmainyout->addLayout(layout1, 1);
    hmainyout->addWidget(titleLabel, 1);
    hmainyout->addStretch(1);

    hmainyout->setMargin(0);
    hmainyout->setSpacing(0);

    setLayout(hmainyout);

    // passing out return signal.
    connect(btnReturn, SIGNAL(clicked(bool)), this, SIGNAL(returnClick()));
}

TopWidgets::~TopWidgets()
{
}
