#include "languagewidgets.h"
#include <QVBoxLayout>

LanguageWidgets::LanguageWidgets(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(33, 36, 43);

    initData();
    initLayout();
}

void LanguageWidgets::initData()
{
    m_languageForm = new LanguageForm(this);
}

void LanguageWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;
    vmainlyout->addSpacing(30);
    vmainlyout->addWidget(m_languageForm);
    vmainlyout->addStretch(0);

    // set layout in middle.
    QHBoxLayout *hmainlyout = new QHBoxLayout;
    hmainlyout->addStretch(1);
    hmainlyout->addLayout(vmainlyout, 4);
    hmainlyout->addStretch(1);
    setLayout(hmainlyout);
}
