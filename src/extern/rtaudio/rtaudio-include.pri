include(../../utils.pri)

LIBS *= $$libOutPath(../extern/rtaudio)
INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD
PRE_TARGETDEPS *= $$libPreTargetDeps(../extern/rtaudio)

include(rtaudio-platform.pri)
