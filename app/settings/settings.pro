#-------------------------------------------------
#
# Project created by QtCreator 2017-06-30T10:13:11
#
#-------------------------------------------------
QT += bluetooth network sql dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = settings
TEMPLATE = app

CONFIG+= qt warn_on release

DEFINES += CONFIG_CTRL_IFACE

DEFINES += DEVICE_EVB

LIBS += -lasound
QMAKE_CXXFLAGS = -fpermissive

INCLUDEPATH +=$$PWD main
include(main/main.pri)

INCLUDEPATH +=$$PWD base
include(base/base.pri)

INCLUDEPATH +=$$PWD top
include(top/top.pri)

INCLUDEPATH +=$$PWD middle
include(middle/middle.pri)

INCLUDEPATH +=$$PWD wlan
include(wlan/wlan.pri)

INCLUDEPATH +=$$PWD hotspot
include(hotspot/hotspot.pri)

INCLUDEPATH +=$$PWD bluetooth
include(bluetooth/bluetooth.pri)

INCLUDEPATH +=$$PWD brightness
include(brightness/brightness.pri)

#INCLUDEPATH +=$$PWD calendar
#include (calendar/calendar.pri)

INCLUDEPATH +=$$PWD volume
include (volume/volume.pri)

INCLUDEPATH +=$$PWD updater
include (updater/updater.pri)

INCLUDEPATH +=$$PWD language
include (language/language.pri)

FORMS = base/qkeyboard.ui \
        brightness/brightness.ui \
        calendar/widget.ui \
        volume/volumnwidget.ui \
        updater/updaterwidget.ui \
        language/languageform.ui

RESOURCES += \
    res_setting.qrc \
    i18n.qrc

TRANSLATIONS += translations/i18n_en.ts \
                translations/i18n_zh.ts
