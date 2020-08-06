win32 {
    DEFINES *= __WINDOWS_ASIO__
    LIBS *= -lole32
}

linux {
    DEFINES *= __LINUX_ALSA__
    LIBS *= -lasound
    LIBS *= -lpthread
}
