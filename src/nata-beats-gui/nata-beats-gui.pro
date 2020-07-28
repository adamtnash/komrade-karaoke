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

# RtAudio
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libs/rtaudio/release/ -lrtaudio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libs/rtaudio/debug/ -lrtaudio
else:unix: LIBS += -L$$OUT_PWD/../libs/rtaudio/ -lrtaudio

INCLUDEPATH += $$PWD/../libs/rtaudio
DEPENDPATH += $$PWD/../libs/rtaudio

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtaudio/release/librtaudio.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtaudio/debug/librtaudio.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtaudio/release/rtaudio.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtaudio/debug/rtaudio.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../libs/rtaudio/librtaudio.a

# RtMidi
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libs/rtmidi/release/ -lrtmidi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libs/rtmidi/debug/ -lrtmidi
else:unix: LIBS += -L$$OUT_PWD/../libs/rtmidi/ -lrtmidi

INCLUDEPATH += $$PWD/../libs/rtmidi
DEPENDPATH += $$PWD/../libs/rtmidi

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtmidi/release/librtmidi.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtmidi/debug/librtmidi.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtmidi/release/rtmidi.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libs/rtmidi/debug/rtmidi.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../libs/rtmidi/librtmidi.a



include(../libs/rtaudio/rtaudio-platform.pri)
include(../libs/rtmidi/rtmidi-platform.pri)
