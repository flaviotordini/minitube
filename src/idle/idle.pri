INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src

HEADERS += $$PWD/src/idle.h

mac {
    SOURCES += $$PWD/src/idle_mac.cpp
} else:win32 {
    SOURCES += $$PWD/src/idle_win.cpp
} else {
    QT *= dbus
    SOURCES += $$PWD/src/idle_linux.cpp
}
