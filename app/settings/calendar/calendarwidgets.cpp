#include "calendarwidgets.h"
#include <QVBoxLayout>

CalendarWidgets::CalendarWidgets(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(33, 36, 43);

    initData();
    initLayout();
}

void CalendarWidgets::initData()
{
    m_calendarWidget = new Widget(this);
}

void CalendarWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;
    vmainlyout->addSpacing(30);
    vmainlyout->addWidget(m_calendarWidget);
    vmainlyout->addStretch(0);

    // set layout in middle.
    QHBoxLayout *hmainlyout = new QHBoxLayout;
    hmainlyout->addStretch(1);
    hmainlyout->addLayout(vmainlyout, 4);
    hmainlyout->addStretch(1);
    setLayout(hmainlyout);
}

CalendarWidgets::~CalendarWidgets()
{
}

