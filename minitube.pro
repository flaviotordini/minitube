# If Phonon cannot be found, uncomment the following (and set the correct path)
# INCLUDEPATH += /usr/include/phonon
CONFIG += release
TEMPLATE = app

# Saner string behaviour
# DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_STRICT_ITERATORS
TARGET = minitube
mac { 
    TARGET = Minitube
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
}
QT += network \
    xml \
    phonon
include(src/qtsingleapplication/qtsingleapplication.pri)
include(src/thlibrary/thlibrary.pri)
HEADERS += src/MainWindow.h \
    src/SearchView.h \
    src/MediaView.h \
    src/SettingsView.h \
    src/AboutView.h \
    src/youtubesearch.h \
    src/video.h \
    src/youtubestreamreader.h \
    src/View.h \
    src/searchlineedit.h \
    src/urllineedit.h \
    src/spacer.h \
    src/Constants.h \
    src/iconloader/qticonloader.h \
    src/faderwidget/FaderWidget.h \
    src/ListModel.h \
    src/playlist/PrettyItemDelegate.h \
    src/networkaccess.h \
    src/videomimedata.h \
    src/global.h \
    src/updatechecker.h \
    src/videowidget.h \
    src/playlistwidget.h \
    src/searchparams.h \
    src/minisplitter.h \
    src/loadingwidget.h
SOURCES += src/main.cpp \
    src/MainWindow.cpp \
    src/SearchView.cpp \
    src/MediaView.cpp \
    src/SettingsView.cpp \
    src/AboutView.cpp \
    src/youtubesearch.cpp \
    src/youtubestreamreader.cpp \
    src/searchlineedit.cpp \
    src/urllineedit.cpp \
    src/spacer.cpp \
    src/video.cpp \
    src/iconloader/qticonloader.cpp \
    src/faderwidget/FaderWidget.cpp \
    src/ListModel.cpp \
    src/playlist/PrettyItemDelegate.cpp \
    src/videomimedata.cpp \
    src/updatechecker.cpp \
    src/videowidget.cpp \
    src/networkaccess.cpp \
    src/playlistwidget.cpp \
    src/searchparams.cpp \
    src/minisplitter.cpp \
    src/loadingwidget.cpp
RESOURCES += resources.qrc
DESTDIR = build/target/
OBJECTS_DIR = build/obj/
MOC_DIR = build/moc/
RCC_DIR = build/rcc/

# Tell Qt Linguist that we use UTF-8 strings in our sources
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
include(locale/locale.pri)

# deploy
DISTFILES += CHANGES \
    LICENSE
mac { 
    CONFIG += x86 \
        ppc
    QMAKE_INFO_PLIST = Info.plist
    ICON = minitube.icns
}
unix { 
    isEmpty(PREFIX):PREFIX = /usr/local
    BINDIR = $$PREFIX/bin
    INSTALLS += target
    target.path = $$BINDIR
    DATADIR = $$PREFIX/share
    PKGDATADIR = $$DATADIR/minitube
    DEFINES += DATADIR=\\\"$$DATADIR\\\" \
        PKGDATADIR=\\\"$$PKGDATADIR\\\"
}
