TEMPLATE = subdirs

SUBDIRS += \
    komrade-karaoke-gui \
    extern/rtmidi \
    extern/rtaudio \
    audiomanager \
    midimanager \
    trackdata

audiomanager.depends = extern/rtaudio trackdata
midimanager.depends = extern/rtmidi
komrade-karaoke-gui.depends = trackdata audiomanager midimanager
