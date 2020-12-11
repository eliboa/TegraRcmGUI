QT       += core gui network
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

# Default config is x64
!contains(CONFIG, ARCH64):!contains(CONFIG, ARCH32) { CONFIG += ARCH64 }

OUT_PWD = $$OUT_PWD
STATIC_BUILD = $$find(OUT_PWD, "Static")
!isEmpty(STATIC_BUILD) { DEFINES += QUAZIP_STATIC }

SOURCES += \
    kourou/kourou.cpp \
    kourou/libs/rcm_device.cpp \
    main.cpp \
    packagemanager.cpp \
    packages.cpp \
    qhekate.cpp \
    qkourou.cpp \
    qobjects/custombutton.cpp \
    qobjects/hekate_ini.cpp \
    qpayload.cpp \
    qprogress_widget.cpp \
    qsettings.cpp \
    qtools.cpp \
    qutils.cpp \
    tegrarcmgui.cpp

HEADERS += \
    kourou/kourou.h \
    kourou/libs/libusbk_int.h \
    kourou/libs/rcm_device.h \
    kourou/usb_command.h \
    packagemanager.h \
    packages.h \
    qerror.h \
    qhekate.h \
    qkourou.h \
    qobjects/custombutton.h \
    qobjects/hekate_ini.h \
    qpayload.h \
    qprogress_widget.h \
    qsettings.h \
    qtools.h \
    qutils.h \
    rcm_device.h \
    tegrarcmgui.h \
    types.h \

FORMS += \
    packagemanager.ui \
    qhekate.ui \
    qpayload.ui \
    qprogress_widget.ui \
    qsettings.ui \
    qtools.ui \
    tegrarcmgui.ui \

TRANSLATIONS = languages/tegrarcmgui_fr.ts

# libusbK (dll)
INCLUDEPATH += $$PWD/libs/libusbK/includes
CONFIG(ARCH64) { LIBS += -L$$PWD/libs/libusbK/bin/lib/amd64/ -llibusbK }
CONFIG(ARCH32) { LIBS += -L$$PWD/libs/libusbK/bin/lib/x86/ -llibusbK }

# quazip & zlib
CMAKE_CXXFLAGS += -std=gnu++14
INCLUDEPATH += $$PWD/libs/zip_libs/include
CONFIG(ARCH64) {
    !isEmpty(STATIC_BUILD) {
        LIBS += -L$$PWD/libs/zip_libs/lib_x64_static/ -lquazip -lz
    } else {
        LIBS += -L$$PWD/libs/zip_libs/lib_x64/ -lquazip -lz
    }
}
CONFIG(ARCH32) {
    !isEmpty(STATIC_BUILD) {
        LIBS += -L$$PWD/libs/zip_libs/lib_x86_static/ -lquazip -lz
    } else {
        LIBS += -L$$PWD/libs/zip_libs/lib_x86/ -lquazip -lz
    }
}

LIBS += -lsetupapi

RESOURCES += \
    qresources.qrc

