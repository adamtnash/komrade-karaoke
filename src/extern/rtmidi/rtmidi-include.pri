include(../../utils.pri)

LIBS *= $$libOutPath(../extern/rtmidi)
INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD
PRE_TARGETDEPS *= $$libPreTargetDeps(../extern/rtmidi)

include(rtmidi-platform.pri)
