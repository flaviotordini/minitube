CONFIG += c++17 exceptions_off rtti_off object_parallel_to_source

TEMPLATE = app
VERSION = 4.0
DEFINES += APP_VERSION="$$VERSION"

APP_NAME = Minitube
DEFINES += APP_NAME="$$APP_NAME"

APP_UNIX_NAME = minitube
DEFINES += APP_UNIX_NAME="$$APP_UNIX_NAME"

message(Building $${APP_NAME} $${VERSION})
message(Qt $$[QT_VERSION] in $$[QT_INSTALL_PREFIX])

DEFINES += APP_SNAPSHOT

CONFIG -= debug_and_release
CONFIG(debug, debug|release): {
    message(Building for debug)
}
CONFIG(release, debug|release): {
    message(Building for release)
    DEFINES *= QT_NO_DEBUG_OUTPUT
    CONFIG += optimize_full
}

DEFINES *= QT_USE_QSTRINGBUILDER

#!contains(DEFINES, APP_GOOGLE_API_KEY=.+) {
#    warning("You need to specify a Google API Key, refer to the README.md file for details")
#}

TARGET = $${APP_UNIX_NAME}

QT += widgets network sql qml

include(lib/qt-reusable-widgets/qt-reusable-widgets.pri)
include(lib/http/http.pri)
include(lib/idle/idle.pri)
include(lib/js/js.pri)
include(lib/promises/promises.pri)
include(lib/yt/yt.pri)

DEFINES += MEDIA_MPV
include(lib/media/media.pri)

!mac {
    DEFINES += QAPPLICATION_CLASS=QApplication
    include(lib/singleapplication/singleapplication.pri)
}

INCLUDEPATH += $$PWD/src

HEADERS += \
    src/constants.h \
    src/playlistitemdelegate.h \
    src/subscriptionimportview.h \
    src/videomimedata.h \
    src/loadingwidget.h \
    src/autocomplete.h \
    src/globalshortcuts.h \
    src/globalshortcutbackend.h \
    src/downloadmanager.h \
    src/downloaditem.h \
    src/downloadview.h \
    src/downloadmodel.h \
    src/downloadlistview.h \
    src/downloadsettings.h \
    src/suggester.h \
    src/channelsuggest.h \
    src/playlistview.h \
    src/refinesearchwidget.h \
    src/refinesearchbutton.h \
    src/sidebarwidget.h \
    src/homeview.h \
    src/aboutview.h \
    src/mainwindow.h \
    src/mediaview.h \
    src/searchview.h \
    src/playlistmodel.h \
    src/waitingspinnerwidget.h \
    src/standardfeedsview.h \
    src/ytregions.h \
    src/ytsuggester.h \
    src/videosourcewidget.h \
    src/regionsview.h \
    src/sidebarheader.h \
    src/gridwidget.h \
    src/database.h \
    src/channelaggregator.h \
    src/channelmodel.h \
    src/aggregatevideosource.h \
    src/channelview.h \
    src/channelitemdelegate.h \
    src/snapshotsettings.h \
    src/snapshotpreview.h \
    src/datautils.h \
    src/searchwidget.h \
    src/channellistview.h \
    src/ytchannel.h \
    src/httputils.h \
    src/toolbarmenu.h \
    src/sharetoolbar.h \
    src/videoarea.h \
    src/searchlineedit.h
SOURCES += src/main.cpp \
    src/subscriptionimportview.cpp \
    src/videomimedata.cpp \
    src/loadingwidget.cpp \
    src/autocomplete.cpp \
    src/constants.cpp \
    src/globalshortcuts.cpp \
    src/globalshortcutbackend.cpp \
    src/downloadmanager.cpp \
    src/downloaditem.cpp \
    src/downloadview.cpp \
    src/downloadmodel.cpp \
    src/downloadlistview.cpp \
    src/downloadsettings.cpp \
    src/channelsuggest.cpp \
    src/playlistview.cpp \
    src/refinesearchwidget.cpp \
    src/refinesearchbutton.cpp \
    src/sidebarwidget.cpp \
    src/homeview.cpp \
    src/mainwindow.cpp \
    src/mediaview.cpp \
    src/aboutview.cpp \
    src/searchview.cpp \
    src/playlistitemdelegate.cpp \
    src/playlistmodel.cpp \
    src/waitingspinnerwidget.cpp \
    src/standardfeedsview.cpp \
    src/ytregions.cpp \
    src/ytsuggester.cpp \
    src/videosourcewidget.cpp \
    src/regionsview.cpp \
    src/sidebarheader.cpp \
    src/gridwidget.cpp \
    src/database.cpp \
    src/channelaggregator.cpp \
    src/channelmodel.cpp \
    src/aggregatevideosource.cpp \
    src/channelview.cpp \
    src/channelitemdelegate.cpp \
    src/snapshotsettings.cpp \
    src/snapshotpreview.cpp \
    src/datautils.cpp \
    src/channellistview.cpp \
    src/ytchannel.cpp \
    src/httputils.cpp \
    src/toolbarmenu.cpp \
    src/sharetoolbar.cpp \
    src/videoarea.cpp \
    src/searchlineedit.cpp

RESOURCES += resources.qrc
RESOURCES += $$files(icons/*.png, true)

DESTDIR = build/target/
OBJECTS_DIR = build/obj/
MOC_DIR = build/moc/
RCC_DIR = build/rcc/

# Tell Qt Linguist that we use UTF-8 strings in our sources
CODECFORTR = UTF-8
CODECFORSRC = UTF-8

include(locale/locale.pri)

# deploy
DISTFILES += CHANGES COPYING
unix:!mac {
    DEFINES += APP_LINUX
    QT += dbus
    HEADERS += src/gnomeglobalshortcutbackend.h
    SOURCES += src/gnomeglobalshortcutbackend.cpp

    isEmpty(PREFIX):PREFIX = /usr

    BINDIR = $$PREFIX/bin
    INSTALLS += target
    target.path = $$BINDIR

    DATADIR = $$PREFIX/share
    PKGDATADIR = $$DATADIR/minitube
    DEFINES += DATADIR=\\\"$$DATADIR\\\" \
        PKGDATADIR=\\\"$$PKGDATADIR\\\"

    INSTALLS += translations \
        sounds \
        desktop \
        appdata \
        iconsvg \
        icon16 \
        icon22 \
        icon32 \
        icon48 \
        icon64 \
        icon128 \
        icon256 \
        icon512
    translations.path = $$PKGDATADIR
    translations.files += $$DESTDIR/locale
    sounds.path = $$PKGDATADIR
    sounds.files += sounds/
    desktop.path = $$DATADIR/applications
    desktop.files += minitube.desktop
    appdata.path = $$DATADIR/metainfo
    appdata.files += org.tordini.flavio.minitube.metainfo.xml
    iconsvg.path = $$DATADIR/icons/hicolor/scalable/apps
    iconsvg.files += data/minitube.svg
    icon16.path = $$DATADIR/icons/hicolor/16x16/apps
    icon16.files += data/16x16/minitube.png
    icon22.path = $$DATADIR/icons/hicolor/22x22/apps
    icon22.files += data/22x22/minitube.png
    icon32.path = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files += data/32x32/minitube.png
    icon48.path = $$DATADIR/icons/hicolor/48x48/apps
    icon48.files += data/48x48/minitube.png
    icon64.path = $$DATADIR/icons/hicolor/64x64/apps
    icon64.files += data/64x64/minitube.png
    icon128.path = $$DATADIR/icons/hicolor/128x128/apps
    icon128.files += data/128x128/minitube.png
    icon256.path = $$DATADIR/icons/hicolor/256x256/apps
    icon256.files += data/256x256/minitube.png
    icon512.path = $$DATADIR/icons/hicolor/512x512/apps
    icon512.files += data/512x512/minitube.png
}

mac|win32|contains(DEFINES, APP_UBUNTU):include(local/local.pri)

message(QT: $$QT)
message(CONFIG: $$CONFIG)
message(DEFINES: $$DEFINES)
message(QMAKE_CXXFLAGS: $$QMAKE_CXXFLAGS)
message(QMAKE_LFLAGS: $$QMAKE_LFLAGS)
