QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

RC_ICONS = res/TegraRcmGUI.ico

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
    kourou/kourou.cpp \
    kourou/libs/rcm_device.cpp \
    main.cpp \
    qkourou.cpp \
    qpayload.cpp \
    qtools.cpp \
    qutils.cpp \
    tegrarcmgui.cpp

HEADERS += \
    kourou/kourou.h \
    kourou/libs/libusbk_int.h \
    kourou/libs/rcm_device.h \
    kourou/usb_command.h \
    qkourou.h \
    qpayload.h \
    qtools.h \
    qutils.h \
    rcm_device.h \
    tegrarcmgui.h \
    types.h

FORMS += \
    qpayload.ui \
    qtools.ui \
    tegrarcmgui.ui

TRANSLATIONS = languages/tegrarcmgui_fr.ts

LIBS += -L$$PWD/../../../../../../../libusbK-dev-kit/bin/lib/amd64/ -llibusbK
INCLUDEPATH += $$PWD/../../../../../../../libusbK-dev-kit/includes
DEPENDPATH += $$PWD/../../../../../../../libusbK-dev-kit/includes

RESOURCES += \
    qresources.qrc

DISTFILES +=
