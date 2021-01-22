#include "pairedinformationdialog.h"

#include <QHBoxLayout>

#include "retranslatemanager.h"

#ifdef DEVICE_EVB
int paired_information_button_width = 150;
int paired_information_button_height = 60;
int paired_information_title_width = 120;
int paired_information_laytout_margin = 40;
#else
int paired_information_button_width = 90;
int paired_information_button_height = 30;
int paired_information_title_width = 80;
int paired_information_laytout_margin = 30;
#endif

PairedInformationDialog::PairedInformationDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setStyleSheet("QDialog{border:1.5px solid rgb(0,120,215);background:rgb(43,43,43)}"
                  "QLabel{color:white}");

    initLayout();
    initConnection();
}

void PairedInformationDialog::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    QLabel *nameTitle = new QLabel(str_bluetooth_name, this);
    nameTitle->setAlignment(Qt::AlignRight);
    nameTitle->setFixedWidth(paired_information_title_width);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setAlignment(Qt::AlignLeft);

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(nameTitle);
    nameLayout->addWidget(m_nameLabel);

    QLabel *addressTitle = new QLabel(str_bluetooth_address, this);
    addressTitle->setAlignment(Qt::AlignRight);
    addressTitle->setFixedWidth(paired_information_title_width);

    m_addressLabel = new QLabel(this);
    m_addressLabel->setAlignment(Qt::AlignLeft);

    QHBoxLayout *addressLayout = new QHBoxLayout;
    addressLayout->addWidget(addressTitle);
    addressLayout->addWidget(m_addressLabel);

    // button layout
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    m_btnConfirm = new CPushButton(str_bluetooth_confirm, this);
    m_btnCancelSave = new CPushButton(str_bluetooth_cancel_save, this);
    m_btnConfirm->setFixedSize(paired_information_button_width, paired_information_button_height);
    m_btnCancelSave->setFixedSize(paired_information_button_width, paired_information_button_height);

    buttonlayout->addWidget(m_btnCancelSave);
    buttonlayout->addWidget(m_btnConfirm);
    buttonlayout->setSpacing(paired_information_laytout_margin);

    mainLayout->addLayout(nameLayout);
    mainLayout->addLayout(addressLayout);
    mainLayout->addSpacing(paired_information_laytout_margin);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setSpacing(10);
    mainLayout->setMargin(paired_information_laytout_margin);
    setLayout(mainLayout);
}

void PairedInformationDialog::initConnection()
{
    connect(m_btnConfirm, SIGNAL(clicked()), this, SLOT(slot_onConfirmClicked()));
    connect(m_btnCancelSave, SIGNAL(clicked()), this, SLOT(slot_onCancelSaveClicked()));
}

int PairedInformationDialog::showInformationDialog(QWidget *parent, QString name, QString address)
{
    PairedInformationDialog *dialog = new PairedInformationDialog(parent);
    dialog->setInformationText(name, address);

    return dialog->exec();
}

void PairedInformationDialog::setInformationText(QString name, QString address)
{
    m_nameLabel->setText(name);
    m_addressLabel->setText(address);
}

int PairedInformationDialog::exec()
{
    this->setWindowModality(Qt::WindowModal);
    this->show();
    m_eventLoop = new QEventLoop(this);
    m_eventLoop->exec();
    return m_chooseResult;
}

void PairedInformationDialog::slot_onConfirmClicked()
{
    m_chooseResult = RESULT_CONFIRM;
    close();
}

void PairedInformationDialog::slot_onCancelSaveClicked()
{
    m_chooseResult = RESULT_CANCELSAVE;
    close();
}

void PairedInformationDialog::closeEvent(QCloseEvent*)
{
    if (m_eventLoop != NULL)
        m_eventLoop->exit();
}
