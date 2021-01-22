#include "updaterwidgets.h"
#include <QVBoxLayout>

UpdaterWidgets::UpdaterWidgets(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(33, 36, 43);

    initData();
    initLayout();
}

void UpdaterWidgets::initData()
{
    m_updaterWidget = new UpdaterWidget(this);
}

void UpdaterWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;
    vmainlyout->addSpacing(30);
    vmainlyout->addWidget(m_updaterWidget);
    vmainlyout->addStretch(0);

    // set layout in middle.
    QHBoxLayout *hmainlyout = new QHBoxLayout;
    hmainlyout->addStretch(1);
    hmainlyout->addLayout(vmainlyout,4);
    hmainlyout->addStretch(1);
    setLayout(hmainlyout);
}

UpdaterWidgets::~UpdaterWidgets()
{
}
