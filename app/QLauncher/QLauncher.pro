lessThan(QT_VERSION, "5.5.0") {
    error("Qt 5.5.0 or above is required.")
}

TEMPLATE = app

TARGET = QLauncher

QT += qml widgets network

# 3399Linux
DEFINES += DEVICE_EVB

QML_IMPORT_PATH = $$PWD/qml

HEADERS += $$files($$PWD/src/*.h) \
           src/powerManager/PowerManager.h \
    src/mediaMonitor/MediaNotification.h \
    src/mediaMonitor/MediaNotificationSender.h \
    src/mediaMonitor/MediaMonitor.h
SOURCES += $$files($$PWD/src/*.cpp) \
           src/powerManager/PowerManager.cpp \
    src/mediaMonitor/MediaNotificationSender.cpp \
    src/mediaMonitor/MediaMonitor.cpp

RESOURCES += resources.qrc

SUBDIRS += \
    src

# exists(  $$PWD/../../device/rockchip/rk3399 ) {
DEFINES += PLATFORM_WAYLAND
# message( "build $$TARGET with $$DEFINES support.")
#   }

