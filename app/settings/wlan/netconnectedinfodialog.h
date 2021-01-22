#ifndef NETCONNECTEDINFODIALOG_H
#define NETCONNECTEDINFODIALOG_H

#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QEventLoop>

#include "basewidget.h"
#include "cpushbutton.h"

class InfoItem : public BaseWidget
{
public:
    InfoItem(QWidget *parent, QString itemName, QString itemValue);
    void setItemValue(QString value) {valueLabel->setText(value);}

private:
    QLabel *valueLabel;

};

class NetConnectedInfoDialog : public QDialog
{
    Q_OBJECT
public:
    NetConnectedInfoDialog(QWidget *parent = 0);
    int static showDialog(QWidget *parent, QString name, QString signalString);

    void setItemInfo(QString name, QString signal);
    int exec();

    enum ChosseResult
    {
        RESULT_COMPLETE,
        RESULT_CANCEL_SAVE
    };

private:
    QLabel *nameLabel;
    CPushButton *m_confirmButton;
    CPushButton *m_cancelSaveButton;
    InfoItem *signalItem;

    QEventLoop* m_eventLoop;
    ChosseResult m_chooseResult;

    void initLayout();
    void initConnection();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void slot_onConfirmClicked();
    void slot_onCancelSaveClicked();
};

#endif // NETCONNECTEDINFODIALOG_H
