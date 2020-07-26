QT       += core gui widgets

CONFIG += c++11

TARGET = nata_beats

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiofilebuffer.cpp \
    main.cpp \
    mainwindow.cpp \
    midimanager.cpp \
    midimessagedelegate.cpp \
    midimessagedialog.cpp \
    midimessageselect.cpp \
    playbackdisplay.cpp \
    playbackmanager.cpp \
    settings.cpp \
    trackcomboboxdelegate.cpp \
    trackdata.cpp \
    trackdatacache.cpp \
    trackfoldermodel.cpp

HEADERS += \
    audiofilebuffer.h \
    mainwindow.h \
    midimanager.h \
    midimessagedelegate.h \
    midimessagedialog.h \
    midimessageselect.h \
    playbackdisplay.h \
    playbackmanager.h \
    settings.h \
    trackcomboboxdelegate.h \
    trackdata.h \
    trackdatacache.h \
    trackfoldermodel.h

FORMS += \
    mainwindow.ui \
    midimessagedialog.ui \
    midimessageselect.ui

#RtAudio and MIDI
INCLUDEPATH += rtaudio rtmidi
SOURCES += \
    rtaudio/RtAudio.cpp \
    rtmidi/RtMidi.cpp

HEADERS += \
    rtaudio/RtAudio.h \
    rtmidi/RtMidi.h

win32 {
    DEFINES += __WINDOWS_ASIO__
    DEFINES += __WINDOWS_MM__
    LIBS += -lole32 -lwinmm

    INCLUDEPATH += rtaudio/include
    SOURCES += $$files(rtaudio/include/*.cpp)
    HEADERS += $$files(rtaudio/include/*.h)
}

unix {
    DEFINES += __UNIX_JACK__
    LIBS += -ljack -lpthread
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
