CONFIG += c++17 exceptions_off rtti_off optimize_full object_parallel_to_source

TEMPLATE = app
VERSION = 3.6.5
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
}

DEFINES *= QT_USE_QSTRINGBUILDER QT_STRICT_ITERATORS QT_DEPRECATED_WARNINGS

#!contains(DEFINES, APP_GOOGLE_API_KEY=.+) {
#    warning("You need to specify a Google API Key, refer to the README.md file for details")
#}

TARGET = $${APP_UNIX_NAME}

QT += widgets network sql qml

include(lib/http/http.pri)
include(lib/idle/idle.pri)
include(lib/js/js.pri)

DEFINES += MEDIA_MPV
include(lib/media/media.pri)

include(src/qtsingleapplication/qtsingleapplication.pri)
include(src/invidious/invidious.pri)
include(src/ytjs/ytjs.pri)
include(src/yt/yt.pri)

INCLUDEPATH += $$PWD/src

HEADERS += src/video.h \
    src/messagebar.h \
    src/spacer.h \
    src/constants.h \
    src/playlistitemdelegate.h \
    src/updateutils.h \
    src/videoapi.h \
    src/videomimedata.h \
    src/searchparams.h \
    src/minisplitter.h \
    src/loadingwidget.h \
    src/autocomplete.h \
    src/videodefinition.h \
    src/fontutils.h \
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
    src/temporary.h \
    src/segmentedcontrol.h \
    src/playlistview.h \
    src/refinesearchwidget.h \
    src/refinesearchbutton.h \
    src/sidebarwidget.h \
    src/homeview.h \
    src/aboutview.h \
    src/mainwindow.h \
    src/mediaview.h \
    src/searchview.h \
    src/view.h \
    src/playlistmodel.h \
    src/videosource.h \
    src/waitingspinnerwidget.h \
    src/ytsearch.h \
    src/ytstandardfeed.h \
    src/standardfeedsview.h \
    src/ytregions.h \
    src/ytcategories.h \
    src/ytsuggester.h \
    src/videosourcewidget.h \
    src/regionsview.h \
    src/ytsinglevideosource.h \
    src/sidebarheader.h \
    src/iconutils.h \
    src/diskcache.h \
    src/gridwidget.h \
    src/painterutils.h \
    src/database.h \
    src/channelaggregator.h \
    src/channelmodel.h \
    src/aggregatevideosource.h \
    src/channelview.h \
    src/channelitemdelegate.h \
    src/jsfunctions.h \
    src/seekslider.h \
    src/snapshotsettings.h \
    src/snapshotpreview.h \
    src/datautils.h \
    src/yt3listparser.h \
    src/ytchannel.h \
    src/yt3.h \
    src/paginatedvideosource.h \
    src/searchwidget.h \
    src/channellistview.h \
    src/httputils.h \
    src/appwidget.h \
    src/clickablelabel.h \
    src/ytvideo.h \
    src/toolbarmenu.h \
    src/sharetoolbar.h \
    src/videoarea.h \
    src/searchlineedit.h
SOURCES += src/main.cpp \
    src/messagebar.cpp \
    src/spacer.cpp \
    src/updateutils.cpp \
    src/video.cpp \
    src/videomimedata.cpp \
    src/searchparams.cpp \
    src/minisplitter.cpp \
    src/loadingwidget.cpp \
    src/autocomplete.cpp \
    src/videodefinition.cpp \
    src/constants.cpp \
    src/fontutils.cpp \
    src/globalshortcuts.cpp \
    src/globalshortcutbackend.cpp \
    src/downloadmanager.cpp \
    src/downloaditem.cpp \
    src/downloadview.cpp \
    src/downloadmodel.cpp \
    src/downloadlistview.cpp \
    src/downloadsettings.cpp \
    src/channelsuggest.cpp \
    src/temporary.cpp \
    src/segmentedcontrol.cpp \
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
    src/videosource.cpp \
    src/waitingspinnerwidget.cpp \
    src/ytsearch.cpp \
    src/ytstandardfeed.cpp \
    src/standardfeedsview.cpp \
    src/ytregions.cpp \
    src/ytcategories.cpp \
    src/ytsuggester.cpp \
    src/videosourcewidget.cpp \
    src/regionsview.cpp \
    src/ytsinglevideosource.cpp \
    src/sidebarheader.cpp \
    src/iconutils.cpp \
    src/diskcache.cpp \
    src/gridwidget.cpp \
    src/painterutils.cpp \
    src/database.cpp \
    src/channelaggregator.cpp \
    src/channelmodel.cpp \
    src/aggregatevideosource.cpp \
    src/channelview.cpp \
    src/channelitemdelegate.cpp \
    src/jsfunctions.cpp \
    src/seekslider.cpp \
    src/snapshotsettings.cpp \
    src/snapshotpreview.cpp \
    src/datautils.cpp \
    src/yt3listparser.cpp \
    src/ytchannel.cpp \
    src/yt3.cpp \
    src/paginatedvideosource.cpp \
    src/channellistview.cpp \
    src/httputils.cpp \
    src/appwidget.cpp \
    src/clickablelabel.cpp \
    src/ytvideo.cpp \
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
