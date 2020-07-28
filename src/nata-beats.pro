TEMPLATE = subdirs

SUBDIRS += \
    nata-beats-gui \
    extern/rtmidi \
    extern/rtaudio \
    audiomanager \
    midimanager \
    trackdata

audiomanager.depends = extern/rtaudio trackdata
midimanager.depends = extern/rtmidi
nata-beats-gui.depends = trackdata audiomanager midimanager
