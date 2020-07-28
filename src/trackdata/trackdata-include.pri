include(../utils.pri)

LIBS *= $$libOutPath(../trackdata)
INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD
PRE_TARGETDEPS *= $$libPreTargetDeps(../trackdata)
