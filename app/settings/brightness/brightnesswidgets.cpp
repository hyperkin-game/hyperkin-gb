#include "brightnesswidgets.h"
#include <QVBoxLayout>

BrightnessWidgets::BrightnessWidgets(QWidget *parent):BaseWidget(parent)
{
    setBackgroundColor(33, 36, 43);

    initData();
    initLayout();
}

void BrightnessWidgets::initData()
{
    m_brightnessWidget = new Brightness(this);
}

void BrightnessWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;
    vmainlyout->addSpacing(30);
    vmainlyout->addWidget(m_brightnessWidget);
    vmainlyout->addStretch(0);

    // set layout in middle.
    QHBoxLayout *hmainlyout = new QHBoxLayout;
    hmainlyout->addStretch(1);
    hmainlyout->addLayout(vmainlyout, 4);
    hmainlyout->addStretch(1);

    setLayout(hmainlyout);
}

BrightnessWidgets::~BrightnessWidgets()
{
}



