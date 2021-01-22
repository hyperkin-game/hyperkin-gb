INCLUDEPATH +=$$PWD bluez5

HEADERS += \
    $$PWD/bluetoothscannerwidgets.h \
    $$PWD/bluetoothdevicetable.h \
    $$PWD/bluez5/bluez5helper.h \
    $$PWD/bluez5/orgbluezdevice1interface.h \
    $$PWD/bluez5/orgfreedesktopdbusobjectmanagerinterface.h \
    $$PWD/bluez5/orgbluezadapter1interface.h \
    $$PWD/pairedinformationdialog.h

SOURCES += \
    $$PWD/bluetoothscannerwidgets.cpp \
    $$PWD/bluetoothdevicetable.cpp \
    $$PWD/bluez5/bluez5helper.cpp \
    $$PWD/bluez5/orgbluezdevice1interface.cpp \
    $$PWD/bluez5/orgfreedesktopdbusobjectmanagerinterface.cpp \
    $$PWD/bluez5/orgbluezadapter1interface.cpp \
    $$PWD/pairedinformationdialog.cpp

