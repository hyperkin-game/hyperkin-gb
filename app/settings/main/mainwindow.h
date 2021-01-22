#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base/basewindow.h"
#include "settingwidgets.h"

class SettingWidgets;

/**
 * The main window of application.
 * Used for global control.such as keypress response、initial interface etc.
 */
class MainWindow : public BaseWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow(){}

protected:
    // used for disable or enable application when car-reverse event comes.
    void disableApplication();
    void enableApplication();

private:
    SettingWidgets *m_setttingsWid;

    void initData();
    void initLayout();

signals:
    void retranslateUi();

public slots:
    void slot_appQuit();
};

#endif // MAINWINDOW_H
