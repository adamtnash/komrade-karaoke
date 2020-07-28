CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

SOURCES += \
    RtAudio.cpp

HEADERS += \
    RtAudio.h

include(rtaudio-platform.pri)

win32 {
    # ASIO Files
    INCLUDEPATH += include
    SOURCES += $$files(include/*.cpp)
    HEADERS += $$files(include/*.h)
}