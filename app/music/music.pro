#-------------------------------------------------
#
# Project created by QtCreator 2017-06-29T09:18:09
#
#-------------------------------------------------

QT += core gui multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = musicPlayer
TEMPLATE = app

CONFIG	+= qt warn_on

# 3399Linux、Big DPI
DEFINES += DEVICE_EVB

INCLUDEPATH +=$$PWD main
include(main/main.pri)

INCLUDEPATH +=$$PWD base
include(base/base.pri)

INCLUDEPATH +=$$PWD music
include(music/music.pri)

INCLUDEPATH +=$$PWD translations

RESOURCES += \
    res_music.qrc \
    res_main.qrc \
    i18n.qrc

TRANSLATIONS += translations/i18n_en.ts \
                translations/i18n_zh.ts
