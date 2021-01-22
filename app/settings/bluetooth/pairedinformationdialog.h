#ifndef PAIREDINFORMATIONDIALOG_H
#define PAIREDINFORMATIONDIALOG_H

#include <QDialog>
#include <QLabel>

#include <cpushbutton.h>
#include <QEventLoop>

class PairedInformationDialog : public QDialog
{
    Q_OBJECT
public:
    PairedInformationDialog(QWidget *parent);

    enum ChosseResult
    {
        RESULT_CONFIRM,
        RESULT_CANCELSAVE
    };

    int static showInformationDialog(QWidget *parent, QString name, QString address);
    void setInformationText(QString name, QString address);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void initLayout();
    void initConnection();
    int exec();

    CPushButton *m_btnConfirm;
    CPushButton *m_btnCancelSave;
    QLabel *m_nameLabel;
    QLabel *m_addressLabel;

    QEventLoop* m_eventLoop;
    ChosseResult m_chooseResult;

private slots:
    void slot_onConfirmClicked();
    void slot_onCancelSaveClicked();
};

#endif // PAIREDINFORMATIONDIALOG_H
