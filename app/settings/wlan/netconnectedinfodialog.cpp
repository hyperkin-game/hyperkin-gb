#include "netconnectedinfodialog.h"

#include <QVBoxLayout>
#include "constant.h"
#include "retranslatemanager.h"

#ifdef DEVICE_EVB
int net_connected_dialog_height = 400;
int net_connected_dialog_width = 600;
int net_connected_dialog_button_height = 60;
int net_connected_dialog_button_width = 190;
#else
int net_connected_dialog_height = 180;
int net_connected_dialog_width = 400;
int net_connected_dialog_button_height = 30;
int net_connected_dialog_button_width = 90;
#endif


InfoItem::InfoItem(QWidget *parent, QString itemName, QString itemValue) : BaseWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *nameLabel = new QLabel(itemName, this);
    QFont font = nameLabel->font();
    font.setBold(true);
    nameLabel->setFont(font);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    valueLabel = new QLabel(itemValue, this);
    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(nameLabel);
    layout->addWidget(valueLabel);
    layout->setSpacing(5);

    setLayout(layout);
}

NetConnectedInfoDialog::NetConnectedInfoDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setStyleSheet("QDialog{border:1.5px solid rgb(0,120,215);background:rgb(43,43,43)}"
                  "QLabel{color:white}");

    initLayout();
    initConnection();
}

void NetConnectedInfoDialog::initLayout()
{
    setMinimumSize(net_connected_dialog_width, net_connected_dialog_height);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    nameLabel = new QLabel(this);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    BaseWidget::setWidgetFontBold(nameLabel);
    BaseWidget::setWidgetFontSize(nameLabel, font_size_big);

    // detail information
    InfoItem *stateItem = new InfoItem(this, str_wifi_state, str_wifi_item_connected);
    signalItem = new InfoItem(this, str_wifi_signal_strength, "");
    InfoItem *frequencyItem = new InfoItem(this, str_wifi_frequency, "2.4GHZ");

    // button layout.
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    m_cancelSaveButton = new CPushButton(str_wifi_forget,this);
    m_confirmButton = new CPushButton(str_wifi_complete, this);
    m_confirmButton->setFixedSize(net_connected_dialog_button_width, net_connected_dialog_button_height);
    m_cancelSaveButton->setFixedSize(net_connected_dialog_button_width, net_connected_dialog_button_height);

    buttonlayout->addWidget(m_cancelSaveButton);
    buttonlayout->addStretch(0);
    buttonlayout->addWidget(m_confirmButton);

    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(stateItem);
    mainLayout->addWidget(signalItem);
    mainLayout->addWidget(frequencyItem);
    mainLayout->addStretch(0);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    setLayout(mainLayout);
}

void NetConnectedInfoDialog::initConnection()
{
    connect(m_confirmButton, SIGNAL(clicked()), this, SLOT(slot_onConfirmClicked()));
    connect(m_cancelSaveButton, SIGNAL(clicked(bool)), this, SLOT(slot_onCancelSaveClicked()));
}

void NetConnectedInfoDialog::setItemInfo(QString name, QString signal)
{
    nameLabel->setText(name);
    signalItem->setItemValue(signal);
}

int NetConnectedInfoDialog::showDialog(QWidget *parent, QString name, QString signalString)
{
    NetConnectedInfoDialog *dialog = new NetConnectedInfoDialog(parent);
    dialog->setItemInfo(name, signalString);

    return dialog->exec();
}

int NetConnectedInfoDialog::exec()
{
    this->setWindowModality(Qt::WindowModal);
    this->show();
    m_eventLoop = new QEventLoop(this);
    m_eventLoop->exec();
    return m_chooseResult;
}

void NetConnectedInfoDialog::slot_onConfirmClicked()
{
    m_chooseResult = RESULT_COMPLETE;
    close();
}

void NetConnectedInfoDialog::slot_onCancelSaveClicked()
{
    m_chooseResult = RESULT_CANCEL_SAVE;
    close();
}

void NetConnectedInfoDialog::closeEvent(QCloseEvent*)
{
    if(m_eventLoop != NULL)
        m_eventLoop->exit();
}

