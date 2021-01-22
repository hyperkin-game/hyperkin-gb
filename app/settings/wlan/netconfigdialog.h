#ifndef NETCONFIGDIALOG_H
#define NETCONFIGDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QEventLoop>

#include "cpushbutton.h"

class NetConfigDialog : public QDialog
{
    Q_OBJECT
public:
    NetConfigDialog(QWidget *parent = 0);
    int static showDialog(QWidget *parent, QString name);

    void setWifiName(QString name);
    int exec();

    enum ChosseResult
    {
        RESULT_CONNECT,
        RESULT_CANCEL,
        RESULT_CANCEL_SAVE
    };

private:
    QLabel *nameLabel;
    CPushButton *m_confirmButton;
    CPushButton *m_cancelButton;
    CPushButton *m_cancelSaveButton;

    QEventLoop* m_eventLoop;
    ChosseResult m_chooseResult;

    void initLayout();
    void initConnection();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void slot_onConfirmClicked();
    void slot_onCancelClicked();
    void slot_onCancelSaveClicked();
};

#endif // NETCONFIGDIALOG_H
