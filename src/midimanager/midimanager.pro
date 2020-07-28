QT += core gui widgets


TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

SOURCES += \
    midiinmanager.cpp \
    midimessagedelegate.cpp \
    midimessagedialog.cpp \
    midimessageselect.cpp

HEADERS += \
    midiinmanager.h \
    midimessagedelegate.h \
    midimessagedialog.h \
    midimessageselect.h

FORMS += \
    midimessagedialog.ui \
    midimessageselect.ui

include(../extern/rtmidi/rtmidi-include.pri)
