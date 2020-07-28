win32 {
    DEFINES *= __WINDOWS_ASIO__
    LIBS *= -lole32
}

unix {
    DEFINES *= __UNIX_JACK__
    LIBS *= -ljack
    LIBS *= -lpthread
}