win32 {
    DEFINES *= __WINDOWS_MM__
    LIBS *= -lwinmm
}

unix {
    DEFINES *= __UNIX_JACK__
    LIBS *= -ljack
}