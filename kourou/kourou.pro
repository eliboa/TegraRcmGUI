QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        kourou.cpp \
        libs/rcm_device.cpp \
        main.cpp

ARCH = 64

contains( ARCH, 64 ) {
    LIBS += -L$$PWD/../../../../../../../../libusbK-dev-kit/bin/lib/amd64/ -llibusbK
}
contains( ARCH, 32 ) {
    LIBS += -L$$PWD/../../../../../../../../libusbK-dev-kit/bin/lib/x86/ -llibusbK
}
INCLUDEPATH += $$PWD/../../../../../../../../libusbK-dev-kit/includes
DEPENDPATH += $$PWD/../../../../../../../../libusbK-dev-kit/includes

HEADERS += \
    kourou.h \
    libs/libusbk_int.h \
    libs/rcm_device.h \
    usb_command.h
