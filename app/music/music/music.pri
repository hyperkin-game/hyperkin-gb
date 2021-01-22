INCLUDEPATH +=$$PWD top
include(top/top.pri)

INCLUDEPATH +=$$PWD middle
include(middle/middle.pri)

INCLUDEPATH +=$$PWD bottom
include(bottom/bottom.pri)

INCLUDEPATH +=$$PWD player
include(player/player.pri)

HEADERS += \
    $$PWD/musicwidgets.h \

SOURCES += \
    $$PWD/musicwidgets.cpp \
