#include "settingwidgets.h"
#include "constant.h"

#include <QVBoxLayout>

SettingWidgets::SettingWidgets(QWidget *parent) : BaseWidget(parent)
{
    setStyleSheet("QLabel{color:white;}");

    initLayout();
    initConnection();
}


void SettingWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;

    m_topWid = new SettingTopWidgets(this);
    m_middleWid = new SettingMiddleWidgets(this);

    vmainlyout->addWidget(m_topWid);
    vmainlyout->addWidget(m_middleWid);
    vmainlyout->setContentsMargins(0, 0, 0, 0);
    vmainlyout->setSpacing(0);

    setLayout(vmainlyout);
}

void SettingWidgets::initConnection()
{
    connect(m_topWid, SIGNAL(returnClicked()), mainWindow, SLOT(slot_appQuit()));
}

SettingWidgets::~SettingWidgets()
{
}
