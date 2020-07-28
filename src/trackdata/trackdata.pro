QT += core gui widgets


TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

SOURCES += \
    audiofilebuffer.cpp \
    trackcomboboxdelegate.cpp \
    trackdata.cpp \
    trackdatacache.cpp \
    trackfolder.cpp \
    trackfoldermodel.cpp

HEADERS += \
    audiofilebuffer.h \
    trackcomboboxdelegate.h \
    trackdata.h \
    trackdatacache.h \
    trackfolder.h \
    trackfoldermodel.h
