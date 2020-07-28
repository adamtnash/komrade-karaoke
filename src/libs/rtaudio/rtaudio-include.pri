include(../../utils.pri)

LIBS += $$libOutPath(../libs/rtaudio)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
PRE_TARGETDEPS += $$libPreTargetDeps(../libs/rtaudio)

include(rtaudio-platform.pri)
