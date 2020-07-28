include(../../utils.pri)

LIBS += $$libOutPath(../libs/rtmidi)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
PRE_TARGETDEPS += $$libPreTargetDeps(../libs/rtmidi)

include(rtmidi-platform.pri)
