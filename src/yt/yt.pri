INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include($$PWD/invidious/invidious.pri)
include($$PWD/ytjs/ytjs.pri)

HEADERS += \
    $$PWD/searchvideosource.h \
    $$PWD/singlevideosource.h \
    $$PWD/ytthumb.h

SOURCES += \
    $$PWD/searchvideosource.cpp \
    $$PWD/singlevideosource.cpp \
    $$PWD/ytthumb.cpp


