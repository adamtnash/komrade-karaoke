include(../utils.pri)

LIBS *= $$libOutPath(../midimanager)
INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD
PRE_TARGETDEPS *= $$libPreTargetDeps(../midimanager)

include(../extern/rtmidi/rtmidi-include.pri)