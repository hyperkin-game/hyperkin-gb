INCLUDEPATH +=$$PWD top
include(top/top.pri)

INCLUDEPATH +=$$PWD middle
include(middle/middle.pri)

INCLUDEPATH +=$$PWD bottom
include(bottom/bottom.pri)

INCLUDEPATH +=$$PWD player
include(player/player.pri)

HEADERS += \
    $$PWD/videowidgets.h \
    $$PWD/controlsurface.h

SOURCES += \
    $$PWD/videowidgets.cpp \
    $$PWD/controlsurface.cpp


