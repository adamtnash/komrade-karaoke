defineReplace(libPreTargetDeps) {
    libPath = $$1
    libName = $$basename(libPath)

    win32-g++:CONFIG(release, debug|release) {
        return($$OUT_PWD/$${libPath}/release/lib$${libName}.a)
    }
    else:win32-g++:CONFIG(debug, debug|release) {
        return($$OUT_PWD/$${libPath}/debug/lib$${libName}.a)
    }
    else:win32:!win32-g++:CONFIG(release, debug|release) {
        return($$OUT_PWD/$${libPath}/release/$${libName}.lib)
    }
    else:win32:!win32-g++:CONFIG(debug, debug|release) {
        return($$OUT_PWD/$${libPath}/debug/$${libName}.lib)
    }
    else:unix {
        return($$OUT_PWD/$${libPath}/lib$${libName}.a)
    }
}

defineReplace(libOutPath) {
    libPath = $$1
    libName = $$basename(libPath)

    win32:CONFIG(release, debug|release) {
        return(-L$$OUT_PWD/$${libPath}/release -l$${libName})
    }
    else:win32:CONFIG(debug, debug|release) {
        return(-L$$OUT_PWD/$${libPath}/debug -l$${libName})
    }
    else:unix {
        return(-L$$OUT_PWD/$${libPath} -l$${libName})
    }
}