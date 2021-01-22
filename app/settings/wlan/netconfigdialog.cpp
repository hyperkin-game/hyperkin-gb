#include "netconfigdialog.h"

#include <QVBoxLayout>

#include "basewidget.h"
#include "constant.h"
#include "retranslatemanager.h"

#ifdef DEVICE_EVB
int net_config_dialog_height = 400;
int net_config_dialog_width = 600;
int net_config_dialog_button_height = 50;
int net_config_dialog_button_width = 120;
#else
int net_config_dialog_height = 180;
int net_config_dialog_width = 400;
int net_config_dialog_button_height = 30;
int net_config_dialog_button_width = 90;
#endif

NetConfigDialog::NetConfigDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setStyleSheet("QDialog{border:1.5px solid rgb(0,120,215);background:rgb(43,43,43)}"
                  "QLabel{color:white}");

    initLayout();
    initConnection();
}

void NetConfigDialog::initLayout()
{
    setMinimumSize(net_config_dialog_width, net_config_dialog_height);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    nameLabel = new QLabel(this);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    BaseWidget::setWidgetFontBold(nameLabel);
    BaseWidget::setWidgetFontSize(nameLabel, font_size_big);

    // button layout.
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    m_confirmButton = new CPushButton(str_wifi_connect, this);
    m_cancelButton = new CPushButton(str_wifi_cancel, this);
    m_cancelSaveButton = new CPushButton(str_wifi_forget, this);
    m_confirmButton->setFixedSize(net_config_dialog_button_width, net_config_dialog_button_height);
    m_cancelButton->setFixedSize(net_config_dialog_button_width, net_config_dialog_button_height);
    m_cancelSaveButton->setFixedSize(net_config_dialog_button_width, net_config_dialog_button_height);

    buttonlayout->addWidget(m_cancelSaveButton);
    buttonlayout->addStretch(0);
    buttonlayout->addWidget(m_cancelButton);
    buttonlayout->addWidget(m_confirmButton);
    buttonlayout->setSpacing(10);

    mainLayout->addWidget(nameLabel);
    mainLayout->addStretch(0);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    setLayout(mainLayout);
}

void NetConfigDialog::initConnection()
{
    connect(m_confirmButton, SIGNAL(clicked()), this, SLOT(slot_onConfirmClicked()));
    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(slot_onCancelClicked()));
    connect(m_cancelSaveButton, SIGNAL(clicked(bool)), this, SLOT(slot_onCancelSaveClicked()));
}

void NetConfigDialog::setWifiName(QString name)
{
    nameLabel->setText(name);
}

int NetConfigDialog::showDialog(QWidget *parent, QString name)
{
    NetConfigDialog *dialog = new NetConfigDialog(parent);
    dialog->setWifiName(name);

    return dialog->exec();
}

int NetConfigDialog::exec()
{
    this->setWindowModality(Qt::WindowModal);
    this->show();
    m_eventLoop = new QEventLoop(this);
    m_eventLoop->exec();
    return m_chooseResult;
}

void NetConfigDialog::slot_onConfirmClicked()
{
    m_chooseResult = RESULT_CONNECT;
    close();
}

void NetConfigDialog::slot_onCancelClicked()
{
    m_chooseResult = RESULT_CANCEL;
    close();
}

void NetConfigDialog::slot_onCancelSaveClicked()
{
    m_chooseResult = RESULT_CANCEL_SAVE;
    close();
}

void NetConfigDialog::closeEvent(QCloseEvent*)
{
    if(m_eventLoop != NULL)
        m_eventLoop->exit();
}

