#ifndef LANGUAGEFORM_H
#define LANGUAGEFORM_H

#include <QWidget>

namespace Ui {
class LanguageForm;
}

class LanguageForm : public QWidget
{
    Q_OBJECT
public:
    explicit LanguageForm(QWidget *parent = 0);
    ~LanguageForm();

private slots:
    void comboBoxLanguage_currentIndexChanged(int index);

private:
    Ui::LanguageForm *ui;
    QStringList qmFiles;
};

#endif // LANGUAGEFORM_H
