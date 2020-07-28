CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

SOURCES += \
    RtMidi.cpp

HEADERS += \
    RtMidi.h

include(rtmidi-platform.pri)