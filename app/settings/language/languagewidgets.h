#ifndef LANGUAGEWIDGETS_H
#define LANGUAGEWIDGETS_H

#include <QWidget>
#include <basewidget.h>
#include "languageform.h"

class LanguageWidgets : public BaseWidget
{
    Q_OBJECT
public:
    explicit LanguageWidgets(QWidget *parent = 0);

private:
    LanguageForm *m_languageForm;

    void initData();
    void initLayout();
};

#endif // LANGUAGEWIDGETS_H
