#-------------------------------------------------
#
# Project created by QtCreator 2017-06-29T17:30:19
#
#-------------------------------------------------

QT  += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = galleryView
TEMPLATE = app

# 3399Linux、Big DPI
DEFINES += DEVICE_EVB

INCLUDEPATH +=$$PWD main
include(main/main.pri)

INCLUDEPATH +=$$PWD base
include(base/base.pri)

INCLUDEPATH +=$$PWD gallery
include(gallery/gallery.pri)

INCLUDEPATH +=$$PWD translations

RESOURCES += \
    res_gallery.qrc \
    res_main.qrc \
    i18n.qrc

TRANSLATIONS += translations/i18n_en.ts \
                translations/i18n_zh.ts
