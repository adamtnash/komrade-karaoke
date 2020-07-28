QT       += core gui widgets

CONFIG += c++11

TARGET = nata_beats

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settings.cpp

HEADERS += \
    mainwindow.h \
    settings.h

FORMS += \
    mainwindow.ui

include(midi/midi.pri)
include(playback/playback.pri)
include(trackdata/trackdata.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include(../libs/rtaudio/rtaudio-include.pri)
include(../libs/rtmidi/rtmidi-include.pri)
