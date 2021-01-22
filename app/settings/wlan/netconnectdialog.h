#ifndef NETCONNECTDIALOG_H
#define NETCONNECTDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QEventLoop>

#include "cpushbutton.h"

class NetConnectDialog : public QDialog
{
    Q_OBJECT
public:
    NetConnectDialog(QWidget *parent = 0);
    int static showDialog(QWidget *parent, QString name);

    void setWifiName(QString name);
    QString getEditText(){return m_passWordEdit->text();}
    int exec();

    enum ChosseResult
    {
        RESULT_CONNECT,
        RESULT_CANCEL
    };

private:
    QLabel *nameLabel;
    QLineEdit *m_passWordEdit;
    CPushButton *m_confirmButton;
    CPushButton *m_cancelButton;

    QEventLoop* m_eventLoop;
    ChosseResult m_chooseResult;

    void initLayout();
    void initConnection();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void slot_onConfirmClicked();
    void slot_onCancelClicked();

    void slot_onEditTextChanged(QString);
};

#endif // NETCONNECTDIALOG_H
