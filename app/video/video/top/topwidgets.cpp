#include "topwidgets.h"
#include "constant.h"

#include <QHBoxLayout>

TopWidget::TopWidget(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(56, 58, 66);

    initLayout();
    // passing out return signal.
    connect(m_btnreturn, SIGNAL(clicked(bool)), this, SIGNAL(returnClick()));
}

void TopWidget::initLayout()
{
    QHBoxLayout *hmainyout = new QHBoxLayout;

    m_btnreturn = new FourStateButton(return_resource_str, this);
    m_btnreturn->setFixedSize(return_icon_width, return_icon_height);

    QHBoxLayout *lyout1 = new QHBoxLayout;
    lyout1->addWidget(m_btnreturn);
    lyout1->addStretch(0);

    m_titleLabel = new QLabel(tr("videoPlayer"), this);
    BaseWidget::setWidgetFontBold(m_titleLabel);
    BaseWidget::setWidgetFontSize(m_titleLabel, font_size_big);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    hmainyout->addLayout(lyout1, 1);
    hmainyout->addWidget(m_titleLabel, 1);
    hmainyout->addStretch(1);
    hmainyout->setMargin(0);
    hmainyout->setSpacing(0);

    setLayout(hmainyout);
}

void TopWidget::setTitleName(const QString &text)
{
    m_titleLabel->setText(text);
}

TopWidget::~TopWidget()
{
}
