TEMPLATE = subdirs

SUBDIRS += \
    nata-beats-gui \
    libs/rtmidi \
    libs/rtaudio

nata-beats-gui.depends = libs/rtaudio libs/rtmidi
