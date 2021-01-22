#include "netconnectdialog.h"

#include <QVBoxLayout>
#include "constant.h"
#include "retranslatemanager.h"

#ifdef DEVICE_EVB
int net_dialog_height = 400;
int net_dialog_width = 600;
int net_dialog_button_height = 60;
int net_dialog_button_width = 180;
#else
int net_dialog_height = 230;
int net_dialog_width = 330;
int net_dialog_button_height = 30;
int net_dialog_button_width = 90;
#endif

NetConnectDialog::NetConnectDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setStyleSheet("QDialog{border:1.5px solid rgb(0,120,215);background:rgb(43,43,43)}"
                  "QLabel{color:white}");

    initLayout();
    initConnection();
}

void NetConnectDialog::initLayout()
{
    setMinimumSize(net_dialog_width, net_dialog_height);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    nameLabel = new QLabel(this);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    BaseWidget::setWidgetFontBold(nameLabel);
    BaseWidget::setWidgetFontSize(nameLabel, font_size_big);

    QLabel *passwordText = new QLabel(str_wifi_underline_password);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_passWordEdit = new QLineEdit(this);

    // button layout.
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    m_confirmButton = new CPushButton(str_wifi_connect,this);
    m_cancelButton = new CPushButton(str_wifi_cancel,this);
    m_confirmButton->setFixedSize(net_dialog_button_width, net_dialog_button_height);
    m_cancelButton->setFixedSize(net_dialog_button_width, net_dialog_button_height);
    m_confirmButton->setEnabled(false);

    buttonlayout->addStretch(0);
    buttonlayout->addWidget(m_cancelButton);
    buttonlayout->addWidget(m_confirmButton);
    buttonlayout->setSpacing(10);

    mainLayout->addWidget(nameLabel);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(passwordText);
    mainLayout->addWidget(m_passWordEdit);
    mainLayout->addStretch(0);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    setLayout(mainLayout);
}

void NetConnectDialog::initConnection()
{
    connect(m_passWordEdit, SIGNAL(textChanged(QString)), this, SLOT(slot_onEditTextChanged(QString)));
    connect(m_confirmButton, SIGNAL(clicked()), this, SLOT(slot_onConfirmClicked()));
    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(slot_onCancelClicked()));
}

void NetConnectDialog::setWifiName(QString name)
{
    nameLabel->setText(name);
}

void NetConnectDialog::slot_onEditTextChanged(QString text)
{
    if (text.length() >= 8)
        m_confirmButton->setEnabled(true);
    else
        m_confirmButton->setEnabled(false);
}

int NetConnectDialog::showDialog(QWidget *parent, QString name)
{
    NetConnectDialog *dialog = new NetConnectDialog(parent);
    dialog->setWifiName(name);

    return dialog->exec();
}

int NetConnectDialog::exec()
{
    this->setWindowModality(Qt::WindowModal);
    this->show();
    m_eventLoop = new QEventLoop(this);
    m_eventLoop->exec();
    return m_chooseResult;
}

void NetConnectDialog::slot_onConfirmClicked()
{
    m_chooseResult = RESULT_CONNECT;
    close();
}

void NetConnectDialog::slot_onCancelClicked()
{
    m_chooseResult = RESULT_CANCEL;
    close();
}

void NetConnectDialog::closeEvent(QCloseEvent*)
{
    if(m_eventLoop != NULL)
        m_eventLoop->exit();
}
