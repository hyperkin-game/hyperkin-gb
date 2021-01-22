#include "volumewidgets.h"
#include <QVBoxLayout>

VolumeWidgets::VolumeWidgets(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(33, 36, 43);

    initData();
    initLayout();
}

void VolumeWidgets::initData()
{
    m_volumnWidget = new VolumnWidget(this);
}

void VolumeWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;
    vmainlyout->addSpacing(30);
    vmainlyout->addWidget(m_volumnWidget);
    vmainlyout->addStretch(0);

    // set layout in middle.
    QHBoxLayout *hmainlyout = new QHBoxLayout;
    hmainlyout->addStretch(1);
    hmainlyout->addLayout(vmainlyout, 4);
    hmainlyout->addStretch(1);
    setLayout(hmainlyout);
}

VolumeWidgets::~VolumeWidgets()
{
}

