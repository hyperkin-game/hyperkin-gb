#include "topwidget.h"
#include "constant.h"

#include <QHBoxLayout>

TopWidget::TopWidget(QWidget *parent):BaseWidget(parent)
{
    setBackgroundColor(56, 58, 66);

    initLayout();
}

TopWidget::~TopWidget()
{
}

void TopWidget::initLayout()
{
    QHBoxLayout *hmainlyout = new QHBoxLayout;

    FourStateButton *btnReturn = new FourStateButton(return_resource_str, this);
    btnReturn->setFixedSize(return_icon_width, return_icon_height);

    FlatButton *btnIcon = new FlatButton(this);
    btnIcon->setFixedSize(top_icon_size, top_icon_size);
    btnIcon->setBackgroundImage(":/image/gallery/gallery_top_icon.png");

    m_titleLabel = new QLabel(this);
    BaseWidget::setWidgetFontSize(m_titleLabel, font_size_big);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *lyout1 = new QHBoxLayout;
    lyout1->addWidget(btnReturn);
    lyout1->addSpacing(50);
    lyout1->addWidget(btnIcon);
    lyout1->addStretch(0);

    hmainlyout->addLayout(lyout1, 1);
    hmainlyout->addWidget(m_titleLabel, 1);
    hmainlyout->addStretch(1);
    hmainlyout->setMargin(0);

    setLayout(hmainlyout);

    // passing out return signal.
    connect(btnReturn, SIGNAL(clicked(bool)), this, SIGNAL(returnClicked()));
}

void TopWidget::updateTopTitle(QString title)
{
    m_titleLabel->setText(title);
}
