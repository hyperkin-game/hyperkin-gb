#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QEventLoop>
#include "cpushbutton.h"

class CMessageBox : public QDialog
{
    Q_OBJECT
public:
    CMessageBox(QWidget *parent = 0);

public:
    void setBoxText(QString contentText, QString confirmText, QString cancelText);
    int static showCMessageBox(QWidget *parent, QString contentText,
                               QString confirmText, QString cancelText);

    enum ChosseResult
    {
        RESULT_CONFIRM,
        RESULT_CANCEL
    };

protected:
    void closeEvent(QCloseEvent *event);

private:
    void initLayout();
    void initConnection();
    int exec();

private:
    QLabel *m_labelContent;
    CPushButton *m_btnConfirm;
    CPushButton *m_btnCancel;

    QEventLoop* m_eventLoop;
    ChosseResult m_chooseResult;

private slots:
    void slot_onConfirmClicked();
    void slot_onCancelClicked();
};

#endif // MESSAGEBOX_H
