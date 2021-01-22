INCLUDEPATH +=$$PWD top
include(top/top.pri)

INCLUDEPATH +=$$PWD middle
include(middle/middle.pri)

HEADERS += \
    $$PWD/gallerywidgets.h \

SOURCES += \
    $$PWD/gallerywidgets.cpp \
