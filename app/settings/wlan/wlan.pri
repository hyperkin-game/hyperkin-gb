DEFINES += CONFIG_CTRL_IFACE_UNIX

SOURCES += $$PWD/wpa_supplicant-2.5/src/utils/os_unix.c \
    $$PWD/wlanlisttable.cpp \
    $$PWD/netconnectdialog.cpp \
    $$PWD/netconfigdialog.cpp \
    $$PWD/netconnectedinfodialog.cpp \

HEADERS += \
    $$PWD/wlanmainwidget.h \
    $$PWD/wpamsg.h \
    $$PWD/wpamanager.h \
    $$PWD/wpa_supplicant-2.5/src/common/wpa_ctrl.h \
    $$PWD/wpaserviceutil.h \
    $$PWD/wlanlisttable.h \
    $$PWD/netconnectdialog.h \
    $$PWD/netconfigdialog.h \
    $$PWD/netconnectedinfodialog.h \

SOURCES += \
    $$PWD/wlanmainwidget.cpp \
    $$PWD/wpamanager.cpp \
    $$PWD/wpa_supplicant-2.5/src/common/wpa_ctrl.c \
