/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "mainwindow.h"
#include "homeview.h"
#include "searchview.h"
#include "mediaview.h"
#include "aboutview.h"
#include "downloadview.h"
#include "spacer.h"
#include "constants.h"
#include "iconutils.h"
#include "videodefinition.h"
#include "fontutils.h"
#include "globalshortcuts.h"
#include "searchparams.h"
#include "videosource.h"
#include "ytsearch.h"
#ifdef APP_LINUX
#include "gnomeglobalshortcutbackend.h"
#endif
#ifdef Q_OS_MAC
#include "mac_startup.h"
#include "macfullscreen.h"
#include "macsupport.h"
#include "macutils.h"
#endif
#include "downloadmanager.h"
#include "ytsuggester.h"
#include "updatechecker.h"
#include "temporary.h"
#if defined(APP_MAC_SEARCHFIELD) && !defined(APP_MAC_QMACTOOLBAR)
#include "searchlineedit_mac.h"
#else
#include "searchlineedit.h"
#endif
#ifdef APP_MAC_QMACTOOLBAR
#include "mactoolbar.h"
#endif
#include <iostream>
#ifdef APP_EXTRA
#include "extra.h"
#include "updatedialog.h"
#endif
#ifdef APP_ACTIVATION
#include "activation.h"
#include "activationview.h"
#include "activationdialog.h"
#endif
#include "ytregions.h"
#include "regionsview.h"
#include "standardfeedsview.h"
#include "channelaggregator.h"
#include "database.h"
#include "videoareawidget.h"
#include "jsfunctions.h"
#include "seekslider.h"
#include "yt3.h"
#include "httputils.h"

namespace {
static MainWindow *singleton = 0;
}

MainWindow* MainWindow::instance() {
    return singleton;
}

MainWindow::MainWindow() :
    updateChecker(0),
    aboutView(0),
    downloadView(0),
    regionsView(0),
    mainToolBar(0),
    #ifdef APP_PHONON
    mediaObject(0),
    audioOutput(0),
    #endif
    fullscreenFlag(false),
    m_compact(false),
    initialized(false) {

    singleton = this;

#ifdef APP_EXTRA
    Extra::windowSetup(this);
#endif

    // views mechanism
    views = new QStackedWidget();
    views->hide();
    setCentralWidget(views);

    messageLabel = new QLabel();
    messageLabel->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    messageLabel->setStyleSheet("padding:5px;border:1px solid #808080;background:palette(window)");
    messageLabel->hide();
    adjustMessageLabelPosition();
    messageTimer = new QTimer(this);
    messageTimer->setInterval(5000);
    messageTimer->setSingleShot(true);
    connect(messageTimer, SIGNAL(timeout()), SLOT(hideMessage()));

    // views
    homeView = new HomeView(this);
    views->addWidget(homeView);

    // TODO make this lazy
    mediaView = MediaView::instance();
    mediaView->setEnabled(false);
    views->addWidget(mediaView);

    // build ui
    createActions();
    createMenus();
    createToolBars();
    hideToolbar();
    createStatusBar();

    // remove that useless menu/toolbar context menu
    this->setContextMenuPolicy(Qt::NoContextMenu);

    // event filter to block ugly toolbar tooltips
    qApp->installEventFilter(this);

    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    // restore window position
    readSettings();

    // fix stacked widget minimum size
    for (int i = 0; i < views->count(); i++)
        views->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMinimumWidth(0);

    views->show();

    // show the initial view
    showHome(false);

#ifdef APP_ACTIVATION
    if (!Activation::instance().isActivated())
        showActivationView(false);
#endif

    QTimer::singleShot(0, this, SLOT(lazyInit()));
}

void MainWindow::lazyInit() {
#ifdef APP_PHONON
    initPhonon();
#endif
    mediaView->initialize();
#ifdef APP_PHONON
    mediaView->setMediaObject(mediaObject);
#endif
    qApp->processEvents();

    // CLI
    if (qApp->arguments().size() > 1) {
        QString query = qApp->arguments().at(1);
        if (query.startsWith(QLatin1String("--"))) {
            messageReceived(query);
            qApp->quit();
        } else {
            SearchParams *searchParams = new SearchParams();
            searchParams->setKeywords(query);
            showMedia(searchParams);
        }
    }

    // Global shortcuts
    GlobalShortcuts &shortcuts = GlobalShortcuts::instance();
#ifdef APP_LINUX
    if (GnomeGlobalShortcutBackend::IsGsdAvailable())
        shortcuts.setBackend(new GnomeGlobalShortcutBackend(&shortcuts));
#endif
#ifdef Q_OS_MAC
    mac::MacSetup();
#endif
    connect(&shortcuts, SIGNAL(PlayPause()), pauseAct, SLOT(trigger()));
    connect(&shortcuts, SIGNAL(Stop()), this, SLOT(stop()));
    connect(&shortcuts, SIGNAL(Next()), skipAct, SLOT(trigger()));
    connect(&shortcuts, SIGNAL(Previous()), skipBackwardAct, SLOT(trigger()));
    // connect(&shortcuts, SIGNAL(StopAfter()), actionMap.value("stopafterthis"), SLOT(toggle()));

    connect(DownloadManager::instance(), SIGNAL(statusMessageChanged(QString)),
            SLOT(updateDownloadMessage(QString)));
    connect(DownloadManager::instance(), SIGNAL(finished()),
            SLOT(downloadsFinished()));

    setAcceptDrops(true);

    mouseTimer = new QTimer(this);
    mouseTimer->setInterval(5000);
    mouseTimer->setSingleShot(true);
    connect(mouseTimer, SIGNAL(timeout()), SLOT(hideMouse()));

    JsFunctions::instance();

    // Hack to give focus to searchlineedit
    QMetaObject::invokeMethod(views->currentWidget(), "appear");
    View* view = qobject_cast<View *> (views->currentWidget());
    QString desc = view->metadata().value("description").toString();
    if (!desc.isEmpty()) showMessage(desc);

#ifdef APP_INTEGRITY
    if (!Extra::integrityCheck()) {
        deleteLater();
        return;
    }
#endif

    ChannelAggregator::instance()->start();

    checkForUpdate();

    initialized = true;
}

void MainWindow::changeEvent(QEvent *e) {
#ifdef APP_MAC
    if (e->type() == QEvent::WindowStateChange) {
        actionMap.value("minimize")->setEnabled(!isMinimized());
    }
#endif
    QMainWindow::changeEvent(e);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e) {

    if (fullscreenFlag && e->type() == QEvent::MouseMove) {
        const char *className = obj->metaObject()->className();
        const bool isHoveringVideo =
                (className == QLatin1String("QGLWidget")) ||
                (className == QLatin1String("VideoAreaWidget"));

        // qDebug() << obj << mouseEvent->pos() << isHoveringVideo << mediaView->isPlaylistVisible();

        if (isHoveringVideo) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (e);
            const int x = mouseEvent->pos().x();

            if (mediaView->isPlaylistVisible()) {
                if (x > 5) mediaView->setPlaylistVisible(false);
            } else {
                if (x >= 0 && x < 5) mediaView->setPlaylistVisible(true);
            }

#ifndef APP_MAC
            const int y = mouseEvent->pos().y();
            if (mainToolBar->isVisible()) {
                if (y > 5) mainToolBar->setVisible(false);
            } else {
                if (y >= 0 && y < 5) mainToolBar->setVisible(true);
            }
#endif

        }

        // show the normal cursor
        unsetCursor();
        // then hide it again after a few seconds
        mouseTimer->start();
    }

    if (e->type() == QEvent::ToolTip) {
        // kill tooltips
        return true;
    }
    // standard event processing
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::createActions() {
    stopAct = new QAction(IconUtils::icon("media-playback-stop"), tr("&Stop"), this);
    stopAct->setStatusTip(tr("Stop playback and go back to the search view"));
    stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    stopAct->setEnabled(false);
    actionMap.insert("stop", stopAct);
    connect(stopAct, SIGNAL(triggered()), SLOT(stop()));

    skipBackwardAct = new QAction(
                IconUtils::icon("media-skip-backward"),
                tr("P&revious"), this);
    skipBackwardAct->setStatusTip(tr("Go back to the previous track"));
    skipBackwardAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
    skipBackwardAct->setEnabled(false);
    actionMap.insert("previous", skipBackwardAct);
    connect(skipBackwardAct, SIGNAL(triggered()), mediaView, SLOT(skipBackward()));

    skipAct = new QAction(IconUtils::icon("media-skip-forward"), tr("S&kip"), this);
    skipAct->setStatusTip(tr("Skip to the next video"));
    skipAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Right) << QKeySequence(Qt::Key_MediaNext));
    skipAct->setEnabled(false);
    actionMap.insert("skip", skipAct);
    connect(skipAct, SIGNAL(triggered()), mediaView, SLOT(skip()));

    pauseAct = new QAction(IconUtils::icon("media-playback-start"), tr("&Play"), this);
    pauseAct->setStatusTip(tr("Resume playback"));
    pauseAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Space) << QKeySequence(Qt::Key_MediaPlay));
    pauseAct->setEnabled(false);
    actionMap.insert("pause", pauseAct);
    connect(pauseAct, SIGNAL(triggered()), mediaView, SLOT(pause()));

    fullscreenAct = new QAction(IconUtils::icon("view-fullscreen"), tr("&Full Screen"), this);
    fullscreenAct->setStatusTip(tr("Go full screen"));
    QList<QKeySequence> fsShortcuts;
#ifdef APP_MAC
    fsShortcuts << QKeySequence(Qt::CTRL + Qt::META + Qt::Key_F);
#else
    fsShortcuts << QKeySequence(Qt::Key_F11) << QKeySequence(Qt::ALT + Qt::Key_Return);
#endif
    fullscreenAct->setShortcuts(fsShortcuts);
    fullscreenAct->setShortcutContext(Qt::ApplicationShortcut);
    fullscreenAct->setPriority(QAction::LowPriority);
    actionMap.insert("fullscreen", fullscreenAct);
    connect(fullscreenAct, SIGNAL(triggered()), this, SLOT(fullscreen()));

    compactViewAct = new QAction(tr("&Compact Mode"), this);
    compactViewAct->setStatusTip(tr("Hide the playlist and the toolbar"));
#ifdef APP_MAC
    compactViewAct->setShortcut(QKeySequence(Qt::CTRL + Qt::META + Qt::Key_C));
#else
    compactViewAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
#endif
    compactViewAct->setCheckable(true);
    compactViewAct->setChecked(false);
    compactViewAct->setEnabled(false);
    actionMap.insert("compactView", compactViewAct);
    connect(compactViewAct, SIGNAL(toggled(bool)), this, SLOT(compactView(bool)));

    webPageAct = new QAction(tr("Open the &YouTube Page"), this);
    webPageAct->setStatusTip(tr("Go to the YouTube video page and pause playback"));
    webPageAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
    webPageAct->setEnabled(false);
    actionMap.insert("webpage", webPageAct);
    connect(webPageAct, SIGNAL(triggered()), mediaView, SLOT(openWebPage()));

    copyPageAct = new QAction(tr("Copy the YouTube &Link"), this);
    copyPageAct->setStatusTip(tr("Copy the current video YouTube link to the clipboard"));
    copyPageAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    copyPageAct->setEnabled(false);
    actionMap.insert("pagelink", copyPageAct);
    connect(copyPageAct, SIGNAL(triggered()), mediaView, SLOT(copyWebPage()));

    copyLinkAct = new QAction(tr("Copy the Video Stream &URL"), this);
    copyLinkAct->setStatusTip(tr("Copy the current video stream URL to the clipboard"));
    copyLinkAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    copyLinkAct->setEnabled(false);
    actionMap.insert("videolink", copyLinkAct);
    connect(copyLinkAct, SIGNAL(triggered()), mediaView, SLOT(copyVideoLink()));

    findVideoPartsAct = new QAction(tr("Find Video &Parts"), this);
    findVideoPartsAct->setStatusTip(tr("Find other video parts hopefully in the right order"));
    findVideoPartsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    findVideoPartsAct->setEnabled(false);
    connect(findVideoPartsAct, SIGNAL(triggered()), mediaView, SLOT(findVideoParts()));
    actionMap.insert("findVideoParts", findVideoPartsAct);

    removeAct = new QAction(tr("&Remove"), this);
    removeAct->setStatusTip(tr("Remove the selected videos from the playlist"));
    removeAct->setShortcuts(QList<QKeySequence>() << QKeySequence("Del") << QKeySequence("Backspace"));
    removeAct->setEnabled(false);
    actionMap.insert("remove", removeAct);
    connect(removeAct, SIGNAL(triggered()), mediaView, SLOT(removeSelected()));

    moveUpAct = new QAction(tr("Move &Up"), this);
    moveUpAct->setStatusTip(tr("Move up the selected videos in the playlist"));
    moveUpAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up));
    moveUpAct->setEnabled(false);
    actionMap.insert("moveUp", moveUpAct);
    connect(moveUpAct, SIGNAL(triggered()), mediaView, SLOT(moveUpSelected()));

    moveDownAct = new QAction(tr("Move &Down"), this);
    moveDownAct->setStatusTip(tr("Move down the selected videos in the playlist"));
    moveDownAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down));
    moveDownAct->setEnabled(false);
    actionMap.insert("moveDown", moveDownAct);
    connect(moveDownAct, SIGNAL(triggered()), mediaView, SLOT(moveDownSelected()));

    clearAct = new QAction(tr("&Clear Recent Searches"), this);
    clearAct->setMenuRole(QAction::ApplicationSpecificRole);
    clearAct->setShortcuts(QList<QKeySequence>()
                           << QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete)
                           << QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Backspace));
    clearAct->setStatusTip(tr("Clear the search history. Cannot be undone."));
    clearAct->setEnabled(true);
    actionMap.insert("clearRecentKeywords", clearAct);
    connect(clearAct, SIGNAL(triggered()), SLOT(clearRecentKeywords()));

    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setMenuRole(QAction::QuitRole);
    quitAct->setShortcut(QKeySequence(QKeySequence::Quit));
    quitAct->setStatusTip(tr("Bye"));
    actionMap.insert("quit", quitAct);
    connect(quitAct, SIGNAL(triggered()), SLOT(quit()));

    siteAct = new QAction(tr("&Website"), this);
    siteAct->setShortcut(QKeySequence::HelpContents);
    siteAct->setStatusTip(tr("%1 on the Web").arg(Constants::NAME));
    actionMap.insert("site", siteAct);
    connect(siteAct, SIGNAL(triggered()), this, SLOT(visitSite()));

#if !defined(APP_MAC) && !defined(APP_WIN)
    donateAct = new QAction(tr("Make a &Donation"), this);
    donateAct->setStatusTip(tr("Please support the continued development of %1").arg(Constants::NAME));
    actionMap.insert("donate", donateAct);
    connect(donateAct, SIGNAL(triggered()), this, SLOT(donate()));
#endif

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setMenuRole(QAction::AboutRole);
    aboutAct->setStatusTip(tr("Info about %1").arg(Constants::NAME));
    actionMap.insert("about", aboutAct);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    // Invisible actions

    searchFocusAct = new QAction(this);
    searchFocusAct->setShortcut(QKeySequence::Find);
    searchFocusAct->setStatusTip(tr("Search"));
    actionMap.insert("search", searchFocusAct);
    connect(searchFocusAct, SIGNAL(triggered()), this, SLOT(searchFocus()));
    addAction(searchFocusAct);

    volumeUpAct = new QAction(this);
    volumeUpAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Plus));
    actionMap.insert("volume-up", volumeUpAct);
    connect(volumeUpAct, SIGNAL(triggered()), this, SLOT(volumeUp()));
    addAction(volumeUpAct);

    volumeDownAct = new QAction(this);
    volumeDownAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Minus));
    actionMap.insert("volume-down", volumeDownAct);
    connect(volumeDownAct, SIGNAL(triggered()), this, SLOT(volumeDown()));
    addAction(volumeDownAct);

    volumeMuteAct = new QAction(this);
    volumeMuteAct->setIcon(IconUtils::icon("audio-volume-high"));
    volumeMuteAct->setStatusTip(tr("Mute volume"));
    volumeMuteAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_K));
    actionMap.insert("volume-mute", volumeMuteAct);
    connect(volumeMuteAct, SIGNAL(triggered()), SLOT(volumeMute()));
    addAction(volumeMuteAct);

    QAction *definitionAct = new QAction(this);
    definitionAct->setIcon(IconUtils::icon("video-display"));
    definitionAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_D));
    /*
    QMenu *definitionMenu = new QMenu(this);
    foreach (QString definition, VideoDefinition::getDefinitionNames()) {
        definitionMenu->addAction(definition);
    }
    definitionAct->setMenu(definitionMenu);
    */
    actionMap.insert("definition", definitionAct);
    connect(definitionAct, SIGNAL(triggered()), SLOT(toggleDefinitionMode()));
    addAction(definitionAct);

    QAction *action;

    action = new QAction(IconUtils::icon("media-playback-start"), tr("&Manually Start Playing"), this);
    action->setStatusTip(tr("Manually start playing videos"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(setManualPlay(bool)));
    actionMap.insert("manualplay", action);

    action = new QAction(tr("&Downloads"), this);
    action->setStatusTip(tr("Show details about video downloads"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    action->setCheckable(true);
    action->setIcon(IconUtils::icon("document-save"));
    connect(action, SIGNAL(toggled(bool)), SLOT(toggleDownloads(bool)));
    actionMap.insert("downloads", action);

    action = new QAction(tr("&Download"), this);
    action->setStatusTip(tr("Download the current video"));
    action->setShortcut(QKeySequence::Save);
    action->setIcon(IconUtils::icon("document-save"));
    action->setEnabled(false);
    action->setVisible(false);
    action->setPriority(QAction::LowPriority);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(downloadVideo()));
    actionMap.insert("download", action);

#ifdef APP_SNAPSHOT
    action = new QAction(tr("Take &Snapshot"), this);
    action->setShortcut(QKeySequence(Qt::Key_F9));
    action->setEnabled(false);
    actionMap.insert("snapshot", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(snapshot()));
#endif

    action = new QAction(tr("&Subscribe to Channel"), this);
    action->setProperty("originalText", action->text());
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    action->setEnabled(false);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(toggleSubscription()));
    actionMap.insert("subscribe-channel", action);
    mediaView->updateSubscriptionAction(0, false);

    QString shareTip = tr("Share the current video using %1");

    action = new QAction("&Twitter", this);
    action->setStatusTip(shareTip.arg("Twitter"));
    action->setEnabled(false);
    actionMap.insert("twitter", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(shareViaTwitter()));

    action = new QAction("&Facebook", this);
    action->setStatusTip(shareTip.arg("Facebook"));
    action->setEnabled(false);
    actionMap.insert("facebook", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(shareViaFacebook()));

    action = new QAction("&Buffer", this);
    action->setStatusTip(shareTip.arg("Buffer"));
    action->setEnabled(false);
    actionMap.insert("buffer", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(shareViaBuffer()));

    action = new QAction(tr("&Email"), this);
    action->setStatusTip(shareTip.arg(tr("Email")));
    action->setEnabled(false);
    actionMap.insert("email", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(shareViaEmail()));

    action = new QAction(tr("&Close"), this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    actionMap.insert("close", action);
    connect(action, SIGNAL(triggered()), SLOT(close()));

    action = new QAction(Constants::NAME, this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_1));
    actionMap.insert("restore", action);
    connect(action, SIGNAL(triggered()), SLOT(restore()));

    action = new QAction(IconUtils::icon("go-top"), tr("&Float on Top"), this);
    action->setCheckable(true);
    actionMap.insert("ontop", action);
    connect(action, SIGNAL(toggled(bool)), SLOT(floatOnTop(bool)));

    action = new QAction(tr("&Adjust Window Size"), this);
    action->setCheckable(true);
    actionMap.insert("adjustwindowsize", action);
    connect(action, SIGNAL(toggled(bool)), SLOT(adjustWindowSizeChanged(bool)));

    action = new QAction(IconUtils::icon("media-playback-stop"), tr("&Stop After This Video"), this);
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Escape));
    action->setCheckable(true);
    action->setEnabled(false);
    actionMap.insert("stopafterthis", action);
    connect(action, SIGNAL(toggled(bool)), SLOT(showStopAfterThisInStatusBar(bool)));

    action = new QAction(tr("&Report an Issue..."), this);
    actionMap.insert("report-issue", action);
    connect(action, SIGNAL(triggered()), SLOT(reportIssue()));

    action = new QAction(tr("&Refine Search..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    action->setCheckable(true);
    action->setEnabled(false);
    actionMap.insert("refine-search", action);

    action = new QAction(YTRegions::worldwideRegion().name, this);
    actionMap.insert("worldwide-region", action);

    action = new QAction(YTRegions::localRegion().name, this);
    actionMap.insert("local-region", action);

    action = new QAction(tr("More..."), this);
    actionMap.insert("more-region", action);

    action = new QAction(IconUtils::icon("view-list"), tr("&Related Videos"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    action->setStatusTip(tr("Watch videos related to the current one"));
    action->setEnabled(false);
    action->setPriority(QAction::LowPriority);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(relatedVideos()));
    actionMap.insert("related-videos", action);

    action = new QAction(tr("Open in &Browser..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    action->setEnabled(false);
    actionMap.insert("open-in-browser", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(openInBrowser()));

    action = new QAction(IconUtils::icon("safesearch"), tr("Restricted Mode"), this);
    action->setStatusTip(tr("Hide videos that may contain inappropriate content"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_K));
    action->setCheckable(true);
    actionMap.insert("safeSearch", action);

#ifdef APP_MAC_STORE
    action = new QAction(tr("&Love %1? Rate it!").arg(Constants::NAME), this);
    actionMap.insert("app-store", action);
    connect(action, SIGNAL(triggered()), SLOT(rateOnAppStore()));
#endif

#ifdef APP_ACTIVATION
    Extra::createActivationAction(tr("Buy %1...").arg(Constants::NAME));
#endif

    // common action properties
    foreach (QAction *action, actionMap.values()) {
        // add actions to the MainWindow so that they work
        // when the menu is hidden
        addAction(action);
        IconUtils::setupAction(action);
    }
}

void MainWindow::createMenus() {

    fileMenu = menuBar()->addMenu(tr("&Application"));
#ifdef APP_ACTIVATION
    QAction *buyAction = actionMap.value("buy");
    if (buyAction) fileMenu->addAction(buyAction);
#ifndef APP_MAC
    fileMenu->addSeparator();
#endif
#endif
    fileMenu->addAction(clearAct);
#ifndef APP_MAC
    fileMenu->addSeparator();
#endif
    fileMenu->addAction(quitAct);

    QMenu* playbackMenu = menuBar()->addMenu(tr("&Playback"));
    menuMap.insert("playback", playbackMenu);
    playbackMenu->addAction(pauseAct);
    playbackMenu->addAction(stopAct);
    playbackMenu->addAction(actionMap.value("stopafterthis"));
    playbackMenu->addSeparator();
    playbackMenu->addAction(skipAct);
    playbackMenu->addAction(skipBackwardAct);
    playbackMenu->addSeparator();
    playbackMenu->addAction(actionMap.value("manualplay"));
#ifdef APP_MAC
    MacSupport::dockMenu(playbackMenu);
#endif

    playlistMenu = menuBar()->addMenu(tr("&Playlist"));
    menuMap.insert("playlist", playlistMenu);
    playlistMenu->addAction(removeAct);
    playlistMenu->addSeparator();
    playlistMenu->addAction(moveUpAct);
    playlistMenu->addAction(moveDownAct);
    playlistMenu->addSeparator();
    playlistMenu->addAction(actionMap.value("refine-search"));

    QMenu* videoMenu = menuBar()->addMenu(tr("&Video"));
    menuMap.insert("video", videoMenu);
    videoMenu->addAction(actionMap.value("related-videos"));
    videoMenu->addAction(findVideoPartsAct);
    videoMenu->addSeparator();
    videoMenu->addAction(actionMap.value("subscribe-channel"));
#ifdef APP_SNAPSHOT
    videoMenu->addSeparator();
    videoMenu->addAction(actionMap.value("snapshot"));
#endif
    videoMenu->addSeparator();
    videoMenu->addAction(webPageAct);
    videoMenu->addAction(copyLinkAct);
    videoMenu->addAction(actionMap.value("open-in-browser"));
    videoMenu->addAction(actionMap.value("download"));

    QMenu* shareMenu = menuBar()->addMenu(tr("&Share"));
    menuMap.insert("share", shareMenu);
    shareMenu->addAction(copyPageAct);
    shareMenu->addSeparator();
    shareMenu->addAction(actionMap.value("twitter"));
    shareMenu->addAction(actionMap.value("facebook"));
    shareMenu->addAction(actionMap.value("buffer"));
    shareMenu->addSeparator();
    shareMenu->addAction(actionMap.value("email"));

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    menuMap.insert("view", viewMenu);
    viewMenu->addAction(actionMap.value("ontop"));
    viewMenu->addSeparator();
    viewMenu->addAction(compactViewAct);
#ifndef APP_MAC
    viewMenu->addAction(fullscreenAct);
#endif
    viewMenu->addSeparator();
    viewMenu->addAction(actionMap.value("adjustwindowsize"));

#ifdef APP_MAC
    MacSupport::windowMenu(this);
#endif

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(siteAct);
#if !defined(APP_MAC) && !defined(APP_WIN)
    helpMenu->addAction(donateAct);
#endif
    helpMenu->addAction(actionMap.value("report-issue"));
    helpMenu->addAction(aboutAct);

#ifdef APP_MAC_STORE
    helpMenu->addSeparator();
    helpMenu->addAction(actionMap.value("app-store"));
#endif
}

void MainWindow::createToolBars() {

    // Create widgets

    currentTime = new QLabel("00:00");
    currentTime->setFont(FontUtils::small());

#ifdef APP_PHONON_SEEK
    seekSlider = new Phonon::SeekSlider();
    seekSlider->setTracking(true);
    seekSlider->setIconVisible(false);
    seekSlider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
#else
    slider = new SeekSlider(this);
    slider->setEnabled(false);
    slider->setTracking(false);
    slider->setMaximum(1000);
    slider->setOrientation(Qt::Horizontal);
    slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
#endif

#ifdef APP_PHONON
    volumeSlider = new Phonon::VolumeSlider();
    volumeSlider->setMuteVisible(false);
    // qDebug() << volumeSlider->children();
    // status tip for the volume slider
    QSlider* volumeQSlider = volumeSlider->findChild<QSlider*>();
    if (volumeQSlider)
        volumeQSlider->setStatusTip(tr("Press %1 to raise the volume, %2 to lower it").arg(
                                        volumeUpAct->shortcut().toString(QKeySequence::NativeText), volumeDownAct->shortcut().toString(QKeySequence::NativeText)));
    // this makes the volume slider smaller
    volumeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#endif

#if defined(APP_MAC_SEARCHFIELD) && !defined(APP_MAC_QMACTOOLBAR)
    SearchWrapper* searchWrapper = new SearchWrapper(this);
    toolbarSearch = searchWrapper->getSearchLineEdit();
#else
    toolbarSearch = new SearchLineEdit(this);
#endif
    toolbarSearch->setMinimumWidth(toolbarSearch->fontInfo().pixelSize()*15);
    toolbarSearch->setSuggester(new YTSuggester(this));
    connect(toolbarSearch, SIGNAL(search(const QString&)), SLOT(search(const QString&)));
    connect(toolbarSearch, SIGNAL(suggestionAccepted(Suggestion*)), SLOT(suggestionAccepted(Suggestion*)));
    toolbarSearch->setStatusTip(searchFocusAct->statusTip());

    // Add widgets to toolbar

#ifdef APP_MAC_QMACTOOLBAR
    currentTime->hide();
    toolbarSearch->hide();
    volumeSlider->hide();
    seekSlider->hide();
    MacToolbar::instance().createToolbar(this);
    return;
#endif

    mainToolBar = new QToolBar(this);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainToolBar->setFloatable(false);
    mainToolBar->setMovable(false);
    mainToolBar->setIconSize(QSize(32, 32));

    mainToolBar->addAction(stopAct);
    mainToolBar->addAction(pauseAct);
    mainToolBar->addAction(skipAct);
    mainToolBar->addAction(actionMap.value("related-videos"));
    mainToolBar->addAction(actionMap.value("download"));

    bool addFullScreenAct = true;
#ifdef Q_OS_MAC
    addFullScreenAct = !mac::CanGoFullScreen(winId());
#endif
    if (addFullScreenAct) mainToolBar->addAction(fullscreenAct);

    mainToolBar->addWidget(new Spacer());
    mainToolBar->addWidget(currentTime);
    mainToolBar->addWidget(new Spacer());
#ifdef APP_PHONON_SEEK
    mainToolBar->addWidget(seekSlider);
#else
    mainToolBar->addWidget(slider);
#endif

    /*
    mainToolBar->addWidget(new Spacer());
    totalTime = new QLabel(mainToolBar);
    totalTime->setFont(smallerFont);
    mainToolBar->addWidget(totalTime);
    */

    mainToolBar->addWidget(new Spacer());
    mainToolBar->addAction(volumeMuteAct);
#ifdef APP_LINUX
    QToolButton *volumeMuteButton = qobject_cast<QToolButton *>(mainToolBar->widgetForAction(volumeMuteAct));
    volumeMuteButton->setIcon(volumeMuteButton->icon().pixmap(16));
#endif

#ifdef APP_PHONON
    mainToolBar->addWidget(volumeSlider);
#endif

    mainToolBar->addWidget(new Spacer());

#if defined(APP_MAC_SEARCHFIELD) && !defined(APP_MAC_QMACTOOLBAR)
    mainToolBar->addWidget(searchWrapper);
#else
    mainToolBar->addWidget(toolbarSearch);
    mainToolBar->addWidget(new Spacer());
#endif

    addToolBar(mainToolBar);
}

void MainWindow::createStatusBar() {
    statusToolBar = new QToolBar(this);
    statusToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    statusToolBar->setIconSize(QSize(16, 16));

    regionAction = new QAction(this);
    regionAction->setStatusTip(tr("Choose your content location"));

    QAction *localAction = actionMap.value("local-region");
    if (!localAction->text().isEmpty()) {
        QMenu *regionMenu = new QMenu(this);
        regionMenu->addAction(actionMap.value("worldwide-region"));
        regionMenu->addAction(localAction);
        regionMenu->addSeparator();
        QAction *moreRegionsAction = actionMap.value("more-region");
        regionMenu->addAction(moreRegionsAction);
        connect(moreRegionsAction, SIGNAL(triggered()), SLOT(showRegionsView()));
        regionAction->setMenu(regionMenu);
    } else {
        connect(regionAction, SIGNAL(triggered()), SLOT(showRegionsView()));
    }

    /* Stupid code that generates the QRC items
    foreach(YTRegion r, YTRegions::list())
        qDebug() << QString("<file>flags/%1.png</file>").arg(r.id.toLower());
    */

    statusBar()->addPermanentWidget(statusToolBar);
}

void MainWindow::showStopAfterThisInStatusBar(bool show) {
    QAction* action = actionMap.value("stopafterthis");
    showActionInStatusBar(action, show);
}

void MainWindow::showActionInStatusBar(QAction* action, bool show) {
#ifdef APP_EXTRA
    Extra::fadeInWidget(statusBar(), statusBar());
#endif
    if (show) {
        if (statusToolBar->actions().contains(action)) return;
        statusToolBar->insertAction(statusToolBar->actions().first(), action);
        if (statusBar()->isHidden() && !fullscreenFlag)
            setStatusBarVisibility(true);
    } else {
        statusToolBar->removeAction(action);
        if (statusBar()->isVisible() && !needStatusBar())
            setStatusBarVisibility(false);
    }
}

void MainWindow::setStatusBarVisibility(bool show) {
#ifdef APP_MAC
    // workaround Qt bug with garbled floating statusToolBar when statusBar is hidden
    statusToolBar->setVisible(show);
#endif
    statusBar()->setVisible(show);
    if (views->currentWidget() == mediaView)
        QTimer::singleShot(0, mediaView, SLOT(maybeAdjustWindowSize()));
}

void MainWindow::adjustStatusBarVisibility() {
    setStatusBarVisibility(needStatusBar());
}

void MainWindow::hideToolbar() {
#ifdef APP_MAC
    mac::showToolBar(winId(), false);
#else
    mainToolBar->hide();
#endif
}

void MainWindow::showToolbar() {
#ifdef APP_MAC
    mac::showToolBar(winId(), true);
#else
    mainToolBar->show();
#endif
}

void MainWindow::readSettings() {
    QSettings settings;
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
#ifdef APP_MAC
        MacSupport::fixGeometry(this);
#endif
    } else {
        const QRect desktopSize = qApp->desktop()->availableGeometry();
        int w = qMin(2000, desktopSize.width());
        int h = qMin(w / 3, desktopSize.height());
        setGeometry(
                    QStyle::alignedRect(
                        Qt::LeftToRight,
                        Qt::AlignTop,
                        QSize(w, h),
                        desktopSize)
                    );
    }
    const VideoDefinition& firstDefinition = VideoDefinition::getDefinitions().first();
    setDefinitionMode(settings.value("definition", firstDefinition.getName()).toString());
    actionMap.value("manualplay")->setChecked(settings.value("manualplay", false).toBool());
    actionMap.value("adjustwindowsize")->setChecked(settings.value("adjustWindowSize", true).toBool());
    actionMap.value("safeSearch")->setChecked(settings.value("safeSearch", true).toBool());
}

void MainWindow::writeSettings() {
    QSettings settings;

    if (!isReallyFullScreen())
        settings.setValue("geometry", saveGeometry());
    mediaView->saveSplitterState();

#ifdef APP_PHONON
    if (audioOutput->volume() > 0.1)
        settings.setValue("volume", audioOutput->volume());
    // settings.setValue("volumeMute", audioOutput->isMuted());
#endif

    settings.setValue("manualplay", actionMap.value("manualplay")->isChecked());
    settings.setValue("safeSearch", actionMap.value("safeSearch")->isChecked());
}

void MainWindow::goBack() {
    if (history.size() > 1) {
        history.pop();
        QWidget *widget = history.pop();
        showWidget(widget);
    }
}

void MainWindow::showWidget(QWidget* widget, bool transition) {
    Q_UNUSED(transition);

    setUpdatesEnabled(false);

    if (compactViewAct->isChecked())
        compactViewAct->toggle();

    // call hide method on the current view
    View* oldView = qobject_cast<View *> (views->currentWidget());
    if (oldView) {
        oldView->disappear();
        views->currentWidget()->setEnabled(false);
    } else qDebug() << "Cannot cast view";

    const bool isMediaView = widget == mediaView;

    stopAct->setEnabled(isMediaView);
    compactViewAct->setEnabled(isMediaView);
    toolbarSearch->setEnabled(widget == homeView || isMediaView || widget == downloadView);

    aboutAct->setEnabled(widget != aboutView);
    actionMap.value("downloads")->setChecked(widget == downloadView);

    QWidget *oldWidget = views->currentWidget();
    if (oldWidget)
        oldWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    views->setCurrentWidget(widget);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // call show method on the new view
    View* newView = qobject_cast<View *> (widget);
    if (newView) {
        widget->setEnabled(true);
        QHash<QString,QVariant> metadata = newView->metadata();

        QString title = metadata.value("title").toString();
        if (title.isEmpty()) title = Constants::NAME;
        else title += QLatin1String(" - ") + Constants::NAME;
        setWindowTitle(title);

        statusToolBar->setUpdatesEnabled(false);

        // dynamic view actions
        foreach (QAction* action, viewActions)
            showActionInStatusBar(action, false);
        viewActions = newView->getViewActions();
        foreach (QAction* action, viewActions)
            showActionInStatusBar(action, true);

        newView->appear();

        adjustStatusBarVisibility();
        messageLabel->hide();

        statusToolBar->setUpdatesEnabled(true);

        /*
        QString desc = metadata.value("description").toString();
        if (!desc.isEmpty()) showMessage(desc);
        */
    }

    setUpdatesEnabled(true);

    history.push(widget);
}

void MainWindow::about() {
    if (!aboutView) {
        aboutView = new AboutView(this);
        views->addWidget(aboutView);
    }
    showWidget(aboutView);
}

void MainWindow::visitSite() {
    QUrl url(Constants::WEBSITE);
    showMessage(QString(tr("Opening %1").arg(url.toString())));
    QDesktopServices::openUrl(url);
}

void MainWindow::donate() {
    QUrl url(QString(Constants::WEBSITE) + "#donate");
    showMessage(QString(tr("Opening %1").arg(url.toString())));
    QDesktopServices::openUrl(url);
}

void MainWindow::reportIssue() {
    QUrl url("http://flavio.tordini.org/forums/forum/minitube-forums/minitube-troubleshooting");
    QDesktopServices::openUrl(url);
}

void MainWindow::quit() {
#ifdef APP_MAC
    if (!confirmQuit()) {
        return;
    }
#endif
    // do not save geometry when in full screen or in compact mode
    if (!fullscreenFlag && !compactViewAct->isChecked()) {
        writeSettings();
    }
    // mediaView->stop();
    Temporary::deleteAll();
    ChannelAggregator::instance()->stop();
    ChannelAggregator::instance()->cleanup();
    Database::shutdown();
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *e) {
#ifdef APP_MAC
    mac::closeWindow(winId());
    e->ignore();
#else
    if (!confirmQuit()) {
        e->ignore();
        return;
    }
    QWidget::closeEvent(e);
    quit();
#endif
    messageLabel->hide();
}

void MainWindow::showEvent(QShowEvent *e) {
    QWidget::showEvent(e);
#ifdef APP_MAC
    restore();
#endif
}

bool MainWindow::confirmQuit() {
    if (DownloadManager::instance()->activeItems() > 0) {
        QMessageBox msgBox(this);
        msgBox.setIconPixmap(IconUtils::pixmap(":/images/64x64/app.png"));
        msgBox.setText(tr("Do you want to exit %1 with a download in progress?").arg(Constants::NAME));
        msgBox.setInformativeText(tr("If you close %1 now, this download will be cancelled.").arg(Constants::NAME));
        msgBox.setModal(true);
        // make it a "sheet" on the Mac
        msgBox.setWindowModality(Qt::WindowModal);

        msgBox.addButton(tr("Close and cancel download"), QMessageBox::RejectRole);
        QPushButton *waitButton = msgBox.addButton(tr("Wait for download to finish"), QMessageBox::ActionRole);

        msgBox.exec();

        if (msgBox.clickedButton() == waitButton) {
            return false;
        }
    }
    return true;
}

void MainWindow::showHome(bool transition) {
    showWidget(homeView, transition);
    currentTime->clear();
    // totalTime->clear();
}

void MainWindow::showMedia(SearchParams *searchParams) {
    showWidget(mediaView);
    if (actionMap.value("safeSearch")->isChecked())
        searchParams->setSafeSearch(SearchParams::Strict);
    else
        searchParams->setSafeSearch(SearchParams::None);
    mediaView->search(searchParams);
}

void MainWindow::showMedia(VideoSource *videoSource) {
    showWidget(mediaView);
    mediaView->setVideoSource(videoSource);
}

#ifdef APP_PHONON
void MainWindow::stateChanged(Phonon::State newState, Phonon::State /* oldState */) {

    // qDebug() << "Phonon state: " << newState;

    switch (newState) {

    case Phonon::ErrorState:
        if (mediaObject->errorType() == Phonon::FatalError) {
            // Do not display because we try to play incomplete video files and sometimes trigger this
            // We retry automatically (in MediaView) so no need to show it
            // showMessage(tr("Fatal error: %1").arg(mediaObject->errorString()));
        } else {
            showMessage(tr("Error: %1").arg(mediaObject->errorString()));
        }
        break;

    case Phonon::PlayingState:
        pauseAct->setEnabled(true);
        pauseAct->setIcon(IconUtils::icon("media-playback-pause"));
        pauseAct->setText(tr("&Pause"));
        pauseAct->setStatusTip(tr("Pause playback") + " (" +  pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        // stopAct->setEnabled(true);
        break;

    case Phonon::StoppedState:
        pauseAct->setEnabled(false);
        pauseAct->setIcon(IconUtils::icon("media-playback-start"));
        pauseAct->setText(tr("&Play"));
        pauseAct->setStatusTip(tr("Resume playback") + " (" +  pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        // stopAct->setEnabled(false);
        break;

    case Phonon::PausedState:
        pauseAct->setEnabled(true);
        pauseAct->setIcon(IconUtils::icon("media-playback-start"));
        pauseAct->setText(tr("&Play"));
        pauseAct->setStatusTip(tr("Resume playback") + " (" +  pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        // stopAct->setEnabled(true);
        break;

    case Phonon::BufferingState:
        pauseAct->setEnabled(false);
        pauseAct->setIcon(IconUtils::icon("content-loading"));
        pauseAct->setText(tr("&Loading..."));
        pauseAct->setStatusTip(QString());
        break;

    case Phonon::LoadingState:
        pauseAct->setEnabled(false);
        currentTime->clear();
        // totalTime->clear();
        // stopAct->setEnabled(true);
        break;

    default:
        ;
    }
}
#endif

void MainWindow::stop() {
    showHome();
    mediaView->stop();
}

void MainWindow::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);
#ifdef APP_MAC
    if (initialized && mac::CanGoFullScreen(winId())) {
        bool isFullscreen = mac::IsFullScreen(winId());
        if (isFullscreen != fullscreenFlag) {
            if (compactViewAct->isChecked()) {
                compactViewAct->setChecked(false);
                compactView(false);
            }
            fullscreenFlag = isFullscreen;
            updateUIForFullscreen();
        }
    }
#endif
#ifdef APP_MAC_QMACTOOLBAR
    toolbarSearch->move(width() - toolbarSearch->width() - 7, -38);
#endif
    adjustMessageLabelPosition();
}

void MainWindow::moveEvent(QMoveEvent *e) {
    Q_UNUSED(e);
    adjustMessageLabelPosition();
}

void MainWindow::fullscreen() {

    if (compactViewAct->isChecked())
        compactViewAct->toggle();

#ifdef APP_MAC
    WId handle = winId();
    if (mac::CanGoFullScreen(handle)) {
        if (mainToolBar) mainToolBar->setVisible(true);
        mac::ToggleFullScreen(handle);
        return;
    }
#endif

    fullscreenFlag = !fullscreenFlag;

    if (fullscreenFlag) {
        // Enter full screen

        m_maximized = isMaximized();

        // save geometry now, if the user quits when in full screen
        // geometry won't be saved
        writeSettings();

#ifdef APP_MAC
        MacSupport::enterFullScreen(this, views);
#else
        if (mainToolBar) mainToolBar->hide();
        showFullScreen();
#endif

    } else {
        // Exit full screen

#ifdef APP_MAC
        MacSupport::exitFullScreen(this, views);
#else
        if (mainToolBar) mainToolBar->show();
        if (m_maximized) showMaximized();
        else showNormal();
#endif

        // Make sure the window has focus
        activateWindow();

    }

    updateUIForFullscreen();

}

void MainWindow::updateUIForFullscreen() {
    static QList<QKeySequence> fsShortcuts;
    static QString fsText;

    if (fullscreenFlag) {
        fsShortcuts = fullscreenAct->shortcuts();
        fsText = fullscreenAct->text();
        if (fsText.isEmpty()) qDebug() << "[taking Empty!]";
        fullscreenAct->setShortcuts(QList<QKeySequence>(fsShortcuts)
                                    << QKeySequence(Qt::Key_Escape));
        fullscreenAct->setText(tr("Leave &Full Screen"));
        fullscreenAct->setIcon(IconUtils::icon("view-restore"));
        setStatusBarVisibility(false);
    } else {
        fullscreenAct->setShortcuts(fsShortcuts);
        if (fsText.isEmpty()) fsText = "[Empty!]";
        fullscreenAct->setText(fsText);
        fullscreenAct->setIcon(IconUtils::icon("view-fullscreen"));

        if (needStatusBar()) setStatusBarVisibility(true);
    }

    // No compact view action when in full screen
    compactViewAct->setVisible(!fullscreenFlag);
    compactViewAct->setChecked(false);

    // Hide anything but the video
    mediaView->setPlaylistVisible(!fullscreenFlag);
    if (mainToolBar) mainToolBar->setVisible(!fullscreenFlag);

#ifndef APP_MAC
    menuBar()->setVisible(!fullscreenFlag);
#endif

    if (fullscreenFlag) {
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_MediaStop));
    } else {
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    }

#ifdef Q_OS_MAC
    MacSupport::fullScreenActions(actionMap.values(), fullscreenFlag);
#endif

    if (views->currentWidget() == mediaView)
        mediaView->setFocus();

    if (fullscreenFlag) {
        hideMouse();
    } else {
        mouseTimer->stop();
        unsetCursor();
    }
}

bool MainWindow::isReallyFullScreen() {
#ifdef Q_OS_MAC
    WId handle = winId();
    if (mac::CanGoFullScreen(handle)) return mac::IsFullScreen(handle);
    else return isFullScreen();
#else
    return isFullScreen();
#endif
}

void MainWindow::missingKeyWarning() {
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(IconUtils::pixmap(":/images/64x64/app.png"));
    msgBox.setText(QString("%1 was built without a Google API key.").arg(Constants::NAME));
    msgBox.setInformativeText(QString("It won't work unless you enter one."
                              "<p>In alternative you can get %1 from the developer site.").arg(Constants::NAME));
    msgBox.setModal(true);
    msgBox.setWindowModality(Qt::WindowModal);
    QPushButton *enterKeyButton = msgBox.addButton(QString("Enter API key..."), QMessageBox::AcceptRole);
    QPushButton *devButton = msgBox.addButton(QString("Get from %1").arg(Constants::WEBSITE), QMessageBox::AcceptRole);
    QPushButton *helpButton = msgBox.addButton(QMessageBox::Help);
    msgBox.exec();
    if (msgBox.clickedButton() == helpButton) {
        QDesktopServices::openUrl(QUrl("https://github.com/flaviotordini/minitube/blob/master/README.md#google-api-key"));
    } else if (msgBox.clickedButton() == enterKeyButton) {
        bool ok;
        QString text = QInputDialog::getText(this, QString(),
                                             tr("Google API key:"), QLineEdit::Normal,
                                             QString(), &ok);
        if (ok && !text.isEmpty()) {
            QSettings settings;
            settings.setValue("googleApiKey", text);
            YT3::instance().initApiKeys();
        }
    } else if (msgBox.clickedButton() == devButton) {
        QDesktopServices::openUrl(QUrl(Constants::WEBSITE));
    }
}

void MainWindow::compactView(bool enable) {
    m_compact = enable;

    static QList<QKeySequence> compactShortcuts;
    static QList<QKeySequence> stopShortcuts;

    const static QString key = "compactGeometry";
    QSettings settings;

#ifndef APP_MAC
    menuBar()->setVisible(!enable);
#endif

    if (enable) {
        setMinimumSize(320, 180);
#ifdef Q_OS_MAC
        mac::RemoveFullScreenWindow(winId());
#endif
        writeSettings();

        if (settings.contains(key))
            restoreGeometry(settings.value(key).toByteArray());
        else
            resize(320, 180);

#ifdef APP_MAC_QMACTOOLBAR
        mac::showToolBar(winId(), !enable);
#else
        mainToolBar->setVisible(!enable);
#endif
        mediaView->setPlaylistVisible(!enable);
        statusBar()->hide();

        compactShortcuts = compactViewAct->shortcuts();
        stopShortcuts = stopAct->shortcuts();

        QList<QKeySequence> newStopShortcuts(stopShortcuts);
        newStopShortcuts.removeAll(QKeySequence(Qt::Key_Escape));
        stopAct->setShortcuts(newStopShortcuts);
        compactViewAct->setShortcuts(QList<QKeySequence>(compactShortcuts) << QKeySequence(Qt::Key_Escape));

        // ensure focus does not end up to the search box
        // as it would steal the Space shortcut
        mediaView->setFocus();

    } else {
        // unset minimum size
        setMinimumSize(0, 0);
#ifdef Q_OS_MAC
        mac::SetupFullScreenWindow(winId());
#endif
        settings.setValue(key, saveGeometry());
#ifdef APP_MAC_QMACTOOLBAR
        mac::showToolBar(winId(), !enable);
#else
        mainToolBar->setVisible(!enable);
#endif
        mediaView->setPlaylistVisible(!enable);
        if (needStatusBar()) setStatusBarVisibility(true);

        readSettings();

        compactViewAct->setShortcuts(compactShortcuts);
        stopAct->setShortcuts(stopShortcuts);
    }

    // auto float on top
    floatOnTop(enable, false);

#ifdef Q_OS_MAC
    mac::compactMode(winId(), enable);
#endif
}

void MainWindow::searchFocus() {
    toolbarSearch->selectAll();
    toolbarSearch->setFocus();
}

#ifdef APP_PHONON
void MainWindow::initPhonon() {
    // Phonon initialization
    if (mediaObject) delete mediaObject;
    if (audioOutput) delete audioOutput;
    mediaObject = new Phonon::MediaObject(this);
    mediaObject->setTickInterval(100);
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(tick(qint64)), SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(totalTimeChanged(qint64)), SLOT(totalTimeChanged(qint64)));

    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    connect(audioOutput, SIGNAL(volumeChanged(qreal)), SLOT(volumeChanged(qreal)));
    connect(audioOutput, SIGNAL(mutedChanged(bool)), SLOT(volumeMutedChanged(bool)));
    Phonon::createPath(mediaObject, audioOutput);
    volumeSlider->setAudioOutput(audioOutput);

#ifdef APP_PHONON_SEEK
    seekSlider->setMediaObject(mediaObject);
#endif

    QSettings settings;
    audioOutput->setVolume(settings.value("volume", 1.).toReal());
    // audioOutput->setMuted(settings.value("volumeMute").toBool());

    mediaObject->stop();
}
#endif

void MainWindow::tick(qint64 time) {
    const QString s = formatTime(time);
    if (s != currentTime->text()) {
        currentTime->setText(s);
        emit currentTimeChanged(s);
    }

    // remaining time
#ifdef APP_PHONON
    const qint64 remainingTime = mediaObject->remainingTime();
    currentTime->setStatusTip(tr("Remaining time: %1").arg(formatTime(remainingTime)));

#ifndef APP_PHONON_SEEK
    const qint64 totalTime = mediaObject->totalTime();
    slider->blockSignals(true);
    // qWarning() << totalTime << time << time * 100 / totalTime;
    if (totalTime > 0 && time > 0 && !slider->isSliderDown() && mediaObject->state() == Phonon::PlayingState)
        slider->setValue(time * slider->maximum() / totalTime);
    slider->blockSignals(false);
#endif

#endif
}

void MainWindow::totalTimeChanged(qint64 time) {
    if (time <= 0) {
        // totalTime->clear();
        return;
    }
    // totalTime->setText(formatTime(time));

    /*
    slider->blockSignals(true);
    slider->setMaximum(time/1000);
    slider->blockSignals(false);
    */
}

QString MainWindow::formatTime(qint64 duration) {
    duration /= 1000;
    QString res;
    int seconds = (int) (duration % 60);
    duration /= 60;
    int minutes = (int) (duration % 60);
    duration /= 60;
    int hours = (int) (duration % 24);
    if (hours == 0)
        return res.sprintf("%02d:%02d", minutes, seconds);
    return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
}

void MainWindow::volumeUp() {
#ifdef APP_PHONON
    qreal newVolume = volumeSlider->audioOutput()->volume() + .1;
    if (newVolume > volumeSlider->maximumVolume())
        newVolume = volumeSlider->maximumVolume();
    volumeSlider->audioOutput()->setVolume(newVolume);
#endif
}

void MainWindow::volumeDown() {
#ifdef APP_PHONON
    qreal newVolume = volumeSlider->audioOutput()->volume() - .1;
    if (newVolume < 0.)
        newVolume = 0.;
    volumeSlider->audioOutput()->setVolume(newVolume);
#endif
}

void MainWindow::volumeMute() {
#ifdef APP_PHONON
    bool muted = volumeSlider->audioOutput()->isMuted();
    volumeSlider->audioOutput()->setMuted(!muted);
    qApp->processEvents();
    if (muted && volumeSlider->audioOutput()->volume() == 0) {
        volumeSlider->audioOutput()->setVolume(volumeSlider->maximumVolume());
    }
    qDebug() << volumeSlider->audioOutput()->isMuted() << volumeSlider->audioOutput()->volume();
#endif
}

void MainWindow::volumeChanged(qreal newVolume) {
#ifdef APP_PHONON
    // automatically unmute when volume changes
    if (volumeSlider->audioOutput()->isMuted()) volumeSlider->audioOutput()->setMuted(false);

    bool isZero = volumeSlider->property("zero").toBool();
    bool styleChanged = false;
    if (newVolume == 0. && !isZero) {
        volumeSlider->setProperty("zero", true);
        styleChanged = true;
    } else if (newVolume > 0. && isZero) {
        volumeSlider->setProperty("zero", false);
        styleChanged = true;
    }
    if (styleChanged) {
        QSlider* volumeQSlider = volumeSlider->findChild<QSlider*>();
        style()->unpolish(volumeQSlider);
        style()->polish(volumeQSlider);
    }
#endif
    showMessage(tr("Volume at %1%").arg((int)(newVolume*100)));
}

void MainWindow::volumeMutedChanged(bool muted) {
    if (muted) {
        volumeMuteAct->setIcon(IconUtils::icon("audio-volume-muted"));
        showMessage(tr("Volume is muted"));
    } else {
        volumeMuteAct->setIcon(IconUtils::icon("audio-volume-high"));
        showMessage(tr("Volume is unmuted"));
    }
#ifdef APP_LINUX
    QToolButton *volumeMuteButton = qobject_cast<QToolButton *>(mainToolBar->widgetForAction(volumeMuteAct));
    volumeMuteButton->setIcon(volumeMuteButton->icon().pixmap(16));
#endif
}

void MainWindow::setDefinitionMode(const QString &definitionName) {
    QAction *definitionAct = actionMap.value("definition");
    definitionAct->setText(definitionName);
    definitionAct->setStatusTip(tr("Maximum video definition set to %1").arg(definitionAct->text())
                                + " (" +  definitionAct->shortcut().toString(QKeySequence::NativeText) + ")");
    showMessage(definitionAct->statusTip());
    QSettings settings;
    settings.setValue("definition", definitionName);
}

void MainWindow::toggleDefinitionMode() {
    const QString definitionName = QSettings().value("definition").toString();
    const QList<VideoDefinition>& definitions = VideoDefinition::getDefinitions();
    const VideoDefinition& currentDefinition = VideoDefinition::getDefinitionFor(definitionName);
    if (currentDefinition.isEmpty()) {
        setDefinitionMode(definitions.first().getName());
        return;
    }

    int index = definitions.indexOf(currentDefinition);
    if (index != definitions.size() - 1) {
        index++;
    } else {
        index = 0;
    }
    // TODO: pass a VideoDefinition instead of QString.
    setDefinitionMode(definitions.at(index).getName());
}

void MainWindow::showFullscreenToolbar(bool show) {
    if (!fullscreenFlag) return;
    mainToolBar->setVisible(show);
}

void MainWindow::showFullscreenPlaylist(bool show) {
    if (!fullscreenFlag) return;
    mediaView->setPlaylistVisible(show);
}

void MainWindow::clearRecentKeywords() {
    QSettings settings;
    settings.remove("recentKeywords");
    settings.remove("recentChannels");
    if (views->currentWidget() == homeView) {
        SearchView *searchView = homeView->getSearchView();
        searchView->updateRecentKeywords();
        searchView->updateRecentChannels();
    }
    HttpUtils::clearCaches();
    showMessage(tr("Your privacy is now safe"));
}

void MainWindow::setManualPlay(bool enabled) {
    QSettings settings;
    settings.setValue("manualplay", QVariant::fromValue(enabled));
    if (views->currentWidget() == homeView && homeView->currentWidget() == homeView->getSearchView())
        return;
    showActionInStatusBar(actionMap.value("manualplay"), enabled);
}

void MainWindow::updateDownloadMessage(const QString &message) {
    actionMap.value("downloads")->setText(message);
}

void MainWindow::downloadsFinished() {
    actionMap.value("downloads")->setText(tr("&Downloads"));
    showMessage(tr("Downloads complete"));
}

void MainWindow::toggleDownloads(bool show) {

    if (show) {
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_MediaStop));
        actionMap.value("downloads")->setShortcuts(
                    QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_J)
                    << QKeySequence(Qt::Key_Escape));
    } else {
        actionMap.value("downloads")->setShortcuts(
                    QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_J));
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    }

    if (!downloadView) {
        downloadView = new DownloadView(this);
        views->addWidget(downloadView);
    }
    if (show) showWidget(downloadView);
    else goBack();
}

void MainWindow::suggestionAccepted(Suggestion *suggestion) {
    search(suggestion->value);
}

void MainWindow::search(const QString &query) {
    QString q = query.trimmed();
    if (q.length() == 0) return;
    SearchParams *searchParams = new SearchParams();
    searchParams->setKeywords(q);
    showMedia(searchParams);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = e->mimeData()->urls();
        if (urls.isEmpty()) return;
        QUrl url = urls.first();
        QString videoId = YTSearch::videoIdFromUrl(url.toString());
        if (!videoId.isEmpty())
            e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e) {
    if (!toolbarSearch->isEnabled()) return;

    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.isEmpty())
        return;
    QUrl url = urls.first();
    QString videoId = YTSearch::videoIdFromUrl(url.toString());
    if (!videoId.isEmpty()) {
        setWindowTitle(url.toString());
        SearchParams *searchParams = new SearchParams();
        searchParams->setKeywords(videoId);
        showMedia(searchParams);
    }
}

void MainWindow::checkForUpdate() {
    static const QString updateCheckKey = "updateCheck";

    // check every 24h
    QSettings settings;
    uint unixTime = QDateTime::currentDateTime().toTime_t();
    int lastCheck = settings.value(updateCheckKey).toInt();
    int secondsSinceLastCheck = unixTime - lastCheck;
    // qDebug() << "secondsSinceLastCheck" << unixTime << lastCheck << secondsSinceLastCheck;
    if (secondsSinceLastCheck < 86400) return;

    // check it out
    if (updateChecker) delete updateChecker;
    updateChecker = new UpdateChecker();
    connect(updateChecker, SIGNAL(newVersion(QString)),
            this, SLOT(gotNewVersion(QString)));
    updateChecker->checkForUpdate();
    settings.setValue(updateCheckKey, unixTime);
}

void MainWindow::gotNewVersion(const QString &version) {
    if (updateChecker) {
        delete updateChecker;
        updateChecker = 0;
    }

    QSettings settings;
    QString checkedVersion = settings.value("checkedVersion").toString();
    if (checkedVersion == version) return;

#ifdef APP_EXTRA
#ifndef APP_MAC
    UpdateDialog *dialog = new UpdateDialog(version, this);
    dialog->show();
#endif
#else
    simpleUpdateDialog(version);
#endif
}

void MainWindow::simpleUpdateDialog(const QString &version) {
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(IconUtils::pixmap(":/images/64x64/app.png"));
    msgBox.setText(tr("%1 version %2 is now available.").arg(Constants::NAME, version));
    msgBox.setModal(true);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.addButton(QMessageBox::Close);
    QPushButton* laterButton = msgBox.addButton(tr("Remind me later"), QMessageBox::RejectRole);
    QPushButton* updateButton = msgBox.addButton(tr("Update"), QMessageBox::AcceptRole);
    msgBox.exec();
    if (msgBox.clickedButton() != laterButton) {
        QSettings settings;
        settings.setValue("checkedVersion", version);
    }
    if (msgBox.clickedButton() == updateButton) visitSite();
}

bool MainWindow::needStatusBar() {
    return !statusToolBar->actions().isEmpty();
}

void MainWindow::adjustMessageLabelPosition() {
    if (messageLabel->parent() == this)
        messageLabel->move(0, height() - messageLabel->height());
    else
        messageLabel->move(mapToGlobal(QPoint(0, height() - messageLabel->height())));
}

void MainWindow::floatOnTop(bool onTop, bool showAction) {
    if (showAction) showActionInStatusBar(actionMap.value("ontop"), onTop);
#ifdef APP_MAC
    mac::floatOnTop(winId(), onTop);
    return;
#endif
    if (onTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        show();
    } else {
        setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
        show();
    }
}

void MainWindow::adjustWindowSizeChanged(bool enabled) {
    QSettings settings;
    settings.setValue("adjustWindowSize", enabled);
    if (enabled && views->currentWidget() == mediaView)
        mediaView->adjustWindowSize();
}

void MainWindow::restore() {
#ifdef APP_MAC
    mac::uncloseWindow(window()->winId());
#endif
}

void MainWindow::messageReceived(const QString &message) {
    if (message == QLatin1String("--toggle-playing")) {
        if (pauseAct->isEnabled()) pauseAct->trigger();
    } else if (message == QLatin1String("--next")) {
        if (skipAct->isEnabled()) skipAct->trigger();
    } else if (message == QLatin1String("--previous")) {
        if (skipBackwardAct->isEnabled()) skipBackwardAct->trigger();
    } else if (message == QLatin1String("--stop-after-this")) {
        actionMap.value("stopafterthis")->toggle();
    }  else if (message.startsWith("--")) {
        MainWindow::printHelp();
    } else if (!message.isEmpty()) {
        SearchParams *searchParams = new SearchParams();
        searchParams->setKeywords(message);
        showMedia(searchParams);
    }
}

void MainWindow::hideMouse() {
    setCursor(Qt::BlankCursor);
    mediaView->setPlaylistVisible(false);
#ifndef APP_MAC
    mainToolBar->setVisible(false);
#endif
}

#ifdef APP_MAC_STORE
void MainWindow::rateOnAppStore() {
    QDesktopServices::openUrl(QUrl("macappstore://userpub.itunes.apple.com"
                                   "/WebObjects/MZUserPublishing.woa/wa/addUserReview"
                                   "?id=422006190&type=Purple+Software"));
}
#endif

void MainWindow::printHelp() {
    QString msg = QString("%1 %2\n\n").arg(Constants::NAME, Constants::VERSION);
    msg += "Usage: minitube [options]\n";
    msg += "Options:\n";
    msg += "  --toggle-playing\t";
    msg += "Start or pause playback.\n";
    msg += "  --next\t\t";
    msg += "Skip to the next video.\n";
    msg += "  --previous\t\t";
    msg += "Go back to the previous video.\n";
    msg += "  --stop-after-this\t";
    msg += "Stop playback at the end of the video.\n";
    std::cout << msg.toLocal8Bit().data();
}

void MainWindow::showMessage(const QString &message) {
    if (!isVisible()) return;
#ifdef APP_MAC
    if (!mac::isVisible(winId())) return;
#endif
    if (statusBar()->isVisible())
        statusBar()->showMessage(message, 60000);
    else {
        messageLabel->setText(message);
        QSize size = messageLabel->sizeHint();
        // round width to nearest 10 to avoid flicker with fast changing messages (e.g. volume changes)
        int w = size.width();
        const int multiple = 10;
        w = w + multiple/2;
        w -= w % multiple;
        size.setWidth(w);
        messageLabel->resize(size);
        if (messageLabel->isHidden()) {
            adjustMessageLabelPosition();
            messageLabel->show();
        }
        messageTimer->start();
    }
}

void MainWindow::hideMessage() {
    messageLabel->hide();
    messageLabel->clear();
}

#ifdef APP_ACTIVATION
void MainWindow::showActivationView(bool transition) {
    QWidget *activationView = ActivationView::instance();
    if (views->currentWidget() == activationView) {
        buy();
        return;
    }
    views->addWidget(activationView);
    showWidget(activationView, transition);
}

void MainWindow::showActivationDialog() {
    QTimer::singleShot(0, new ActivationDialog(this), SLOT(show()));
}

void MainWindow::buy() {
    Extra::buy();
}

void MainWindow::hideBuyAction() {
    QAction *action = actionMap.value("buy");
    action->setVisible(false);
    action->setEnabled(false);
}
#endif

void MainWindow::showRegionsView() {
    if (!regionsView) {
        regionsView = new RegionsView(this);
        connect(regionsView, SIGNAL(regionChanged()),
                homeView->getStandardFeedsView(), SLOT(load()));
        views->addWidget(regionsView);
    }
    showWidget(regionsView);
}
