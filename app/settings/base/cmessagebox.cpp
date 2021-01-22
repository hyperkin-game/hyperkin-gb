#include "cmessagebox.h"
#include <QHBoxLayout>
#include "constant.h"

#ifdef DEVICE_EVB
int button_width = 150;
int button_height = 60;
int box_margin = 50;
#else
int button_width = 95;
int button_height = 30;
int box_margin = 30;
#endif

CMessageBox::CMessageBox(QWidget *parent) : QDialog(parent)
  , m_eventLoop(NULL)
  , m_chooseResult(RESULT_CANCEL)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setStyleSheet("QDialog{border:1.5px solid rgb(0,120,215);background:rgb(43,43,43)}"
                  "QLabel{color:white}");

    initLayout();
    initConnection();
}

void CMessageBox::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_labelContent = new QLabel(this);
    m_labelContent->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // button layout
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    m_btnConfirm = new CPushButton("", this);
    m_btnCancel = new CPushButton("", this);
    m_btnConfirm->setFixedHeight(button_height);
    m_btnCancel->setFixedHeight(button_height);
    m_btnConfirm->setMinimumWidth(button_width);
    m_btnCancel->setMinimumWidth(button_width);

    buttonlayout->addWidget(m_btnConfirm);
    buttonlayout->addWidget(m_btnCancel);
    buttonlayout->setSpacing(box_margin);

    mainLayout->addWidget(m_labelContent);
    mainLayout->addSpacing(box_margin);
    mainLayout->addLayout(buttonlayout);
    mainLayout->setSpacing(10);
    mainLayout->setMargin(box_margin);
    setLayout(mainLayout);
}

void CMessageBox::initConnection()
{
    connect(m_btnConfirm, SIGNAL(clicked()), this, SLOT(slot_onConfirmClicked()));
    connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(slot_onCancelClicked()));
}

int CMessageBox::showCMessageBox(QWidget *parent, QString contentText,
                                 QString confirmText, QString cancelText)
{
    CMessageBox *messageBox = new CMessageBox(parent);
    messageBox->setBoxText(contentText, confirmText, cancelText);

    return messageBox->exec();
}

void CMessageBox::setBoxText(QString contentText, QString confirmText, QString cancelText)
{
    m_labelContent->setText(contentText);
    m_btnConfirm->setText(confirmText);
    m_btnCancel->setText(cancelText);
}

int CMessageBox::exec()
{
    this->setWindowModality(Qt::WindowModal);
    this->show();
    m_eventLoop = new QEventLoop(this);
    m_eventLoop->exec();
    return m_chooseResult;
}

void CMessageBox::slot_onConfirmClicked()
{
    m_chooseResult = RESULT_CONFIRM;
    close();
}

void CMessageBox::slot_onCancelClicked()
{
    m_chooseResult = RESULT_CANCEL;
    close();
}

void CMessageBox::closeEvent(QCloseEvent*)
{
    if (m_eventLoop != NULL)
        m_eventLoop->exit();
}
