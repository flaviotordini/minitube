QT *= network

INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src

HEADERS += \
    $$PWD/src/cachedhttp.h \
    $$PWD/src/http.h \
    $$PWD/src/localcache.h \
    $$PWD/src/throttledhttp.h

SOURCES += \
    $$PWD/src/cachedhttp.cpp \
    $$PWD/src/http.cpp \
    $$PWD/src/localcache.cpp \
    $$PWD/src/throttledhttp.cpp
