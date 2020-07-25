QT       += core gui widgets

CONFIG += c++11

TARGET = nata_beats

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    RtAudio.cpp \
    RtMidi.cpp \
    audiofilebuffer.cpp \
    main.cpp \
    mainwindow.cpp \
    playbackdisplay.cpp \
    playbackmanager.cpp \
    settings.cpp \
    trackdata.cpp \
    trackdatacache.cpp \
    trackfoldermodel.cpp

HEADERS += \
    RtAudio.h \
    RtMidi.h \
    audiofilebuffer.h \
    mainwindow.h \
    playbackdisplay.h \
    playbackmanager.h \
    settings.h \
    trackdata.h \
    trackdatacache.h \
    trackfoldermodel.h

FORMS += \
    mainwindow.ui

win32 {
    DEFINES += __WINDOWS_DS__
    LIBS += -lole32 -lwinmm -ldsound
}

linux {
    DEFINES += __UNIX_JACK__
    LIBS += -ljack -lpthread
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
