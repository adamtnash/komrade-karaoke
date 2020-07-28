QT += core gui widgets

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

SOURCES += \
    playbackdisplay.cpp \
    playbackmanager.cpp \

HEADERS += \
    playbackdisplay.h \
    playbackmanager.h

include(../extern/rtaudio/rtaudio-include.pri)
include(../trackdata/trackdata-include.pri)
