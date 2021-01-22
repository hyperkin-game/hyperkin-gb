#include "languageform.h"
#include "ui_languageform.h"
#include <QDir>
#include <QDebug>
#include <QTranslator>
#include "language.h"
#include "constant.h"

LanguageForm::LanguageForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LanguageForm)
{
    ui->setupUi(this);

    setStyleSheet("QGroupBox{color:white}");
    ui->comboBoxLanguage->addItem("English", QVariant("en"));
    ui->comboBoxLanguage->addItem("简体中文", QVariant("zh"));


    for (int j = 0; j < ui->comboBoxLanguage->count(); j++) {
        QString lang = ui->comboBoxLanguage->itemData(j).toString();
        if (Language::instance()->languageMatch(lang, Language::instance()->getCurrentQM())) {
            qDebug() << "Current lang:" << lang;
            ui->comboBoxLanguage->setCurrentIndex(j);
        }
    }

    connect(ui->comboBoxLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxLanguage_currentIndexChanged(int)));
}

LanguageForm::~LanguageForm()
{
    delete ui;
}

void LanguageForm::comboBoxLanguage_currentIndexChanged(int index)
{
    Language::instance()->setLang(ui->comboBoxLanguage->itemData(index).toString());

    QTranslator translator;
    translator.load(Language::instance()->getCurrentQM());
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);

    emit mainWindow->retranslateUi();
}
