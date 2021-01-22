#include "settingtopwidgets.h"
#include "constant.h"

SettingTopWidgets::SettingTopWidgets(QWidget *parent):BaseWidget(parent)
{
    setBackgroundColor(56, 58, 66);

    initLayout();
    initConnection();
}

void SettingTopWidgets::initLayout()
{
    QHBoxLayout *hmainyout = new QHBoxLayout;

    QHBoxLayout *returnLayout = new QHBoxLayout;

    m_btnReturn = new FourStateButton(return_resource_str, this);
    m_btnReturn->setFixedSize(return_icon_width, return_icon_height);

    returnLayout->addWidget(m_btnReturn);
    returnLayout->setContentsMargins(0, 0, 0, 8);
    returnLayout->setSpacing(0);

    m_btnIcon = new FlatButton(this);
    m_btnIcon->setFixedSize(top_icon_size, top_icon_size);
    m_btnIcon->setBackgroundImage(":/image/setting/setting_icon.png");
    m_btnIcon->setVisible(true);

    titleText = new QLabel(tr("Setting"), this);
    BaseWidget::setWidgetFontSize(titleText, font_size_big);
    titleText->setAlignment(Qt::AlignCenter);

    QHBoxLayout *lyout1 = new QHBoxLayout;
    lyout1->addLayout(returnLayout);
    lyout1->addSpacing(50);
    lyout1->addWidget(m_btnIcon);
    lyout1->addStretch(0);

    hmainyout->addLayout(lyout1, 1);
    hmainyout->addWidget(titleText, 1);
    hmainyout->addStretch(1);
    hmainyout->setContentsMargins(0, 0, 0, 0);
    setLayout(hmainyout);
}

void SettingTopWidgets::initConnection()
{
    connect(m_btnReturn, SIGNAL(clicked(bool)), this, SIGNAL(returnClicked()));
    connect(mainWindow, SIGNAL(retranslateUi()), this, SLOT(retranslateUi()));
}

void SettingTopWidgets::retranslateUi(){
    titleText->setText(tr("Setting"));
}

SettingTopWidgets::~SettingTopWidgets()
{
}
