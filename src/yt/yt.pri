INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include($$PWD/invidious/invidious.pri)
include($$PWD/ytjs/ytjs.pri)

HEADERS += \
    $$PWD/searchvideosource.h \
    $$PWD/singlevideosource.h

SOURCES += \
    $$PWD/searchvideosource.cpp \
    $$PWD/singlevideosource.cpp


