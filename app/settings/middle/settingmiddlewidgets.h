#ifndef SETTINGMIDDLEWIDGETS_H
#define SETTINGMIDDLEWIDGETS_H

#include <QStackedWidget>

#include "basewidget.h"
#include "leftwidgets.h"
#include "wlan/wlanmainwidget.h"
#include "hotspot/hotspotmainwidget.h"
#include "bluetooth/bluetoothscannerwidgets.h"
#include "brightness/brightnesswidgets.h"
#include "volume/volumewidgets.h"
#include "updater/updaterwidgets.h"
#include "language/languagewidgets.h"

/**
 * On behalf of middle part of settings application.
 * It is made up of 'LeftWidgets' and a 'stackedWidget'
 *
 * Every stacked item has their own ui and logic processing.
 */
class SettingMiddleWidgets : public BaseWidget
{
    Q_OBJECT
public:
    SettingMiddleWidgets(QWidget *parent);
    ~SettingMiddleWidgets();

private:
    /* right stacked widget */
    QStackedWidget *m_stackedWid;
    LeftWidgets *m_leftWid;

    WlanMainWidget *m_wifiWid;
    HotspotMainWidget *m_hotspotWid;
    BluetoothScannerWidgets *m_bluetoothWid;
    BrightnessWidgets *m_brightnessWid;
    VolumeWidgets *m_volumnWid;
    UpdaterWidgets *m_updaterWid;
    LanguageWidgets *m_languageWid;

    void initLayout();
    void initConnection();

private slots:
    void slot_currentWidgetChanged(int index);
};

#endif // MIDDLEWIDGETS_H
