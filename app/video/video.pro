#-------------------------------------------------
#
# Project created by QtCreator 2017-06-29T16:20:46
#
#-------------------------------------------------

QT  +=  multimedia multimediawidgets quickwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = videoPlayer
TEMPLATE = app

# 3399Linux、Big DPI
DEFINES += DEVICE_EVB
DEFINES += PLATFORM_WAYLAND
INCLUDEPATH +=$$PWD main
include(main/main.pri)

INCLUDEPATH +=$$PWD video
include(video/video.pri)

INCLUDEPATH +=$$PWD audioservice
include(audioservice/audioservice.pri)

INCLUDEPATH +=$$PWD base
include(base/base.pri)

RESOURCES += \
    res_main.qrc \
    res_video.qrc \
    qml.qrc \
    i18n.qrc

TRANSLATIONS += translations/i18n_en.ts \
                translations/i18n_zh.ts
