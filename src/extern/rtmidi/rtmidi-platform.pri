
win32 {
    DEFINES *= __WINDOWS_MM__
    LIBS *= -lwinmm
}

linux {
    DEFINES *= __LINUX_ALSA__
    LIBS *= -lasound
    LIBS *= -lpthread
}
