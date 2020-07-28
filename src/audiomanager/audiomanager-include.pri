include(../utils.pri)

LIBS *= $$libOutPath(../audiomanager)
INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD
PRE_TARGETDEPS *= $$libPreTargetDeps(../audiomanager)

include(../extern/rtaudio/rtaudio-include.pri)
include(../trackdata/trackdata-include.pri)