#ifndef HOTSPOTMAINWIDGET_H
#define HOTSPOTMAINWIDGET_H

#include "basewidget.h"
#include "base/switchbutton.h"
#include "lineedititem.h"

#include <QLabel>
#include <QLineEdit>
#include <QThread>

class HotspotThread;

class SwitchHead : public BaseWidget
{
    Q_OBJECT
public:
    SwitchHead(QWidget *parent = 0);
    void setChecked(bool checked);
    void setTitle(const QString &title);

private:
    QLabel *m_switchText;
    SwitchButton *m_switchButton;

signals:
    void sig_checkedChanged(bool);
};

class HotspotMainWidget : public BaseWidget
{
    Q_OBJECT
public:
    HotspotMainWidget(QWidget *parent);

private:
    SwitchHead *m_header;
    LineEditItem *m_nameItem;
    LineEditItem *m_pskItem;
    HotspotThread *m_thread;

    void initLayout();

protected:
    void showEvent(QShowEvent*);

private slots:
    void slot_switchStateChanged(bool checked);
    void retranslateUi();
};

/**
 * Used for start or stop hostapd application.
 */
class HotspotThread : public QThread
{
public:
    HotspotThread(QObject *parent, bool isStartHotspot);
    ~HotspotThread();

private:
    bool isStartHotspot;

protected:
    void run();
};

#endif // HOTSPOTMAINWIDGET_H
