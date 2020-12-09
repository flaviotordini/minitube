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

#include "aboutview.h"
#include "downloadview.h"
#include "homeview.h"
#include "mediaview.h"
#include "regionsview.h"
#include "searchview.h"
#include "standardfeedsview.h"

#include "constants.h"
#include "fontutils.h"
#include "globalshortcuts.h"
#include "iconutils.h"
#include "searchparams.h"
#include "spacer.h"
#include "videodefinition.h"
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
#include "temporary.h"
#include "ytsuggester.h"
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
#include "compositefader.h"
#include "extra.h"
#include "updatedialog.h"
#endif
#ifdef APP_ACTIVATION
#include "activation.h"
#include "activationview.h"
#endif
#include "channelaggregator.h"
#include "database.h"
#include "httputils.h"
#include "jsfunctions.h"
#include "seekslider.h"
#include "sidebarwidget.h"
#include "toolbarmenu.h"
#include "videoarea.h"
#include "yt3.h"
#include "ytregions.h"

#include "invidious.h"
#include "js.h"
#include "videoapi.h"

#ifdef MEDIA_QTAV
#include "mediaqtav.h"
#endif
#ifdef MEDIA_MPV
#include "mediampv.h"
#endif

#ifdef UPDATER
#include "updater.h"
#endif

namespace {
MainWindow *mainWindowInstance;
}

MainWindow *MainWindow::instance() {
    return mainWindowInstance;
}

MainWindow::MainWindow()
    : aboutView(nullptr), downloadView(nullptr), regionsView(nullptr), mainToolBar(nullptr),
      fullScreenActive(false), compactModeActive(false), initialized(false), toolbarMenu(nullptr),
      media(nullptr) {
    mainWindowInstance = this;

    // views mechanism
    views = new QStackedWidget();
    setCentralWidget(views);

#ifdef APP_EXTRA
    Extra::windowSetup(this);
#endif

    messageLabel = new QLabel(this);
    messageLabel->setWordWrap(false);
    messageLabel->setStyleSheet("padding:5px;border:0;background:palette(window)");
    messageLabel->setAlignment(Qt::AlignCenter);
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
    createToolBar();
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

#ifdef APP_ACTIVATION
    Activation::instance().initialCheck();
#else
    showHome();
#endif

    if (VideoAPI::impl() == VideoAPI::IV) {
        Invidious::instance().initServers();
    } else if (VideoAPI::impl() == VideoAPI::YT3) {
        YT3::instance().initApiKeys();
    } else if (VideoAPI::impl() == VideoAPI::JS) {
        JS::instance().getNamFactory().setRequestHeaders(
                {{"User-Agent", HttpUtils::stealthUserAgent()}});
        JS::instance().initialize(QUrl(QLatin1String(Constants::WEBSITE) + "-ws/bundle2.js"));
        Invidious::instance().initServers();
    }

    QTimer::singleShot(100, this, &MainWindow::lazyInit);
}

void MainWindow::lazyInit() {
    mediaView->initialize();
    initMedia();
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
    // connect(&shortcuts, SIGNAL(StopAfter()), getAction("stopafterthis"), SLOT(toggle()));

    connect(DownloadManager::instance(), SIGNAL(statusMessageChanged(QString)),
            SLOT(updateDownloadMessage(QString)));
    connect(DownloadManager::instance(), SIGNAL(finished()), SLOT(downloadsFinished()));

    setAcceptDrops(true);

    fullscreenTimer = new QTimer(this);
    fullscreenTimer->setInterval(3000);
    fullscreenTimer->setSingleShot(true);
    connect(fullscreenTimer, SIGNAL(timeout()), SLOT(hideFullscreenUI()));

    JsFunctions::instance();

    // Hack to give focus to searchlineedit
    View *view = qobject_cast<View *>(views->currentWidget());
    if (view == homeView) {
        QMetaObject::invokeMethod(views->currentWidget(), "appear");
        const QString &desc = view->getDescription();
        if (!desc.isEmpty()) showMessage(desc);
    }

    ChannelAggregator::instance()->start();

#ifdef UPDATER
    Updater::instance().checkWithoutUI();
#endif

    initialized = true;
}

void MainWindow::changeEvent(QEvent *e) {
#ifdef APP_MAC
    if (e->type() == QEvent::WindowStateChange) {
        getAction("minimize")->setEnabled(!isMinimized());
    }
#endif
    if (messageLabel->isVisible()) {
        if (e->type() == QEvent::ActivationChange || e->type() == QEvent::WindowStateChange ||
            e->type() == QEvent::WindowDeactivate || e->type() == QEvent::ApplicationStateChange) {
            hideMessage();
        }
    }
    QMainWindow::changeEvent(e);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e) {
    const QEvent::Type t = e->type();

#ifndef APP_MAC
    static bool altPressed = false;
    if (t == QEvent::KeyRelease && altPressed) {
        altPressed = false;
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Alt) {
            toggleMenuVisibility();
            return true;
        }
    } else if (t == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        altPressed = ke->key() == Qt::Key_Alt;
    }
#endif

    if (fullScreenActive && views->currentWidget() == mediaView && t == QEvent::MouseMove &&
        obj->isWidgetType() && qobject_cast<QWidget *>(obj)->window() == this) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);

        bool toolBarVisible = mainToolBar && mainToolBar->isVisible();
        bool sidebarVisible = mediaView->isSidebarVisible();

        if (!sidebarVisible && !toolBarVisible) {
            const int x = mouseEvent->pos().x();
            if (x >= 0 && x < 5) {
#ifndef APP_MAC
                SidebarWidget *sidebar = mediaView->getSidebar();
                sidebar->resize(sidebar->width(), height());
#endif
                mediaView->setSidebarVisibility(true);
                sidebarVisible = true;
            }
        }

#ifndef APP_MAC
        if (!toolBarVisible && !sidebarVisible) {
            const int y = mouseEvent->pos().y();
            if (y >= 0 && y < 5) {
                mainToolBar->resize(width(), mainToolBar->sizeHint().height());
                mainToolBar->setVisible(true);
            }
        }
#endif

        // show the normal cursor
        unsetCursor();
        // then hide it again after a few seconds
        fullscreenTimer->start();
    }

    if (t == QEvent::ToolTip) {
        // kill tooltips
        return true;
    }

    if (t == QEvent::Show && obj == toolbarMenu) {
#ifdef APP_MAC
        int x = width() - toolbarMenu->sizeHint().width();
        int y = views->y();
#else
        int x = toolbarMenuButton->x() + toolbarMenuButton->width() -
                toolbarMenu->sizeHint().width();
        int y = toolbarMenuButton->y() + toolbarMenuButton->height();
#endif
        QPoint p(x, y);
        toolbarMenu->move(mapToGlobal(p));
    }

    if (obj == this && t == QEvent::StyleChange) {
        qDebug() << "Style change detected";
        qApp->paletteChanged(qApp->palette());
        return false;
    }

    // standard event processing
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::createActions() {
    stopAct = new QAction(tr("&Stop"), this);
    IconUtils::setIcon(stopAct, "media-playback-stop");
    stopAct->setStatusTip(tr("Stop playback and go back to the search view"));
    stopAct->setShortcuts(QList<QKeySequence>()
                          << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    stopAct->setEnabled(false);
    actionMap.insert("stop", stopAct);
    connect(stopAct, SIGNAL(triggered()), SLOT(stop()));

    skipBackwardAct = new QAction(tr("P&revious"), this);
    skipBackwardAct->setStatusTip(tr("Go back to the previous track"));
    skipBackwardAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
    skipBackwardAct->setEnabled(false);
    actionMap.insert("previous", skipBackwardAct);
    connect(skipBackwardAct, SIGNAL(triggered()), mediaView, SLOT(skipBackward()));

    skipAct = new QAction(tr("S&kip"), this);
    IconUtils::setIcon(skipAct, "media-skip-forward");
    skipAct->setStatusTip(tr("Skip to the next video"));
    skipAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Right)
                                                << QKeySequence(Qt::Key_MediaNext));
    skipAct->setEnabled(false);
    actionMap.insert("skip", skipAct);
    connect(skipAct, SIGNAL(triggered()), mediaView, SLOT(skip()));

    pauseAct = new QAction(tr("&Play"), this);
    IconUtils::setIcon(pauseAct, "media-playback-start");
    pauseAct->setStatusTip(tr("Resume playback"));
    pauseAct->setShortcuts(QList<QKeySequence>()
                           << QKeySequence(Qt::Key_Space) << QKeySequence(Qt::Key_MediaPlay));
    pauseAct->setEnabled(false);
    actionMap.insert("pause", pauseAct);
    connect(pauseAct, SIGNAL(triggered()), mediaView, SLOT(pause()));

    fullscreenAct = new QAction(tr("&Full Screen"), this);
    IconUtils::setIcon(fullscreenAct, "view-fullscreen");
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
    connect(fullscreenAct, SIGNAL(triggered()), SLOT(toggleFullscreen()));

    compactViewAct = new QAction(tr("&Compact Mode"), this);
    compactViewAct->setStatusTip(tr("Hide the playlist and the toolbar"));
    compactViewAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
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
    IconUtils::setIcon(copyPageAct, "link");
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
    removeAct->setShortcuts(QList<QKeySequence>()
                            << QKeySequence("Del") << QKeySequence("Backspace"));
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
    donateAct->setStatusTip(
            tr("Please support the continued development of %1").arg(Constants::NAME));
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
    actionMap.insert("volumeUp", volumeUpAct);
    connect(volumeUpAct, SIGNAL(triggered()), this, SLOT(volumeUp()));
    addAction(volumeUpAct);

    volumeDownAct = new QAction(this);
    volumeDownAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Minus));
    actionMap.insert("volumeDown", volumeDownAct);
    connect(volumeDownAct, SIGNAL(triggered()), this, SLOT(volumeDown()));
    addAction(volumeDownAct);

    volumeMuteAct = new QAction(this);
    IconUtils::setIcon(volumeMuteAct, "audio-volume-high");
    volumeMuteAct->setStatusTip(tr("Mute volume"));
    volumeMuteAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_M));
    actionMap.insert("volumeMute", volumeMuteAct);
    connect(volumeMuteAct, SIGNAL(triggered()), SLOT(toggleVolumeMute()));
    addAction(volumeMuteAct);

    QToolButton *definitionButton = new QToolButton(this);
    definitionButton->setText(YT3::instance().maxVideoDefinition().getName());
    IconUtils::setIcon(definitionButton, "video-display");
    definitionButton->setIconSize(QSize(16, 16));
    definitionButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    definitionButton->setPopupMode(QToolButton::InstantPopup);
    QMenu *definitionMenu = new QMenu(this);
    QActionGroup *group = new QActionGroup(this);
    for (auto &defName : VideoDefinition::getDefinitionNames()) {
        QAction *a = new QAction(defName);
        a->setCheckable(true);
        a->setActionGroup(group);
        a->setChecked(defName == YT3::instance().maxVideoDefinition().getName());
        connect(a, &QAction::triggered, this, [this, defName, definitionButton] {
            setDefinitionMode(defName);
            definitionButton->setText(defName);
        });
        connect(&YT3::instance(), &YT3::maxVideoDefinitionChanged, this,
                [defName, definitionButton](const QString &name) {
                    if (defName == name) definitionButton->setChecked(true);
                });
        definitionMenu->addAction(a);
    }
    definitionButton->setMenu(definitionMenu);
    QWidgetAction *definitionAct = new QWidgetAction(this);
    definitionAct->setDefaultWidget(definitionButton);
    definitionAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_D));
    actionMap.insert("definition", definitionAct);
    addAction(definitionAct);

    QAction *action;

    action = new QAction(tr("&Manually Start Playing"), this);
    IconUtils::setIcon(action, "media-playback-start");
    action->setStatusTip(tr("Manually start playing videos"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(setManualPlay(bool)));
    actionMap.insert("manualplay", action);

    action = new QAction(tr("&Downloads"), this);
    IconUtils::setIcon(action, "document-save");
    action->setStatusTip(tr("Show details about video downloads"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(toggleDownloads(bool)));
    actionMap.insert("downloads", action);

    action = new QAction(tr("&Download"), this);
    IconUtils::setIcon(action, "document-save");
    action->setStatusTip(tr("Download the current video"));
    action->setShortcut(QKeySequence::Save);
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
    actionMap.insert("subscribeChannel", action);
    mediaView->updateSubscriptionActionForVideo(0, false);

    QString shareTip = tr("Share the current video using %1");

    action = new QAction("&Twitter", this);
    IconUtils::setIcon(action, "twitter");
    action->setStatusTip(shareTip.arg("Twitter"));
    action->setEnabled(false);
    actionMap.insert("twitter", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(shareViaTwitter()));

    action = new QAction("&Facebook", this);
    IconUtils::setIcon(action, "facebook");
    action->setStatusTip(shareTip.arg("Facebook"));
    action->setEnabled(false);
    actionMap.insert("facebook", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(shareViaFacebook()));

    action = new QAction(tr("&Email"), this);
    IconUtils::setIcon(action, "email");
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

    action = new QAction(tr("&Float on Top"), this);
    IconUtils::setIcon(action, "go-top");
    action->setCheckable(true);
    actionMap.insert("ontop", action);
    connect(action, SIGNAL(toggled(bool)), SLOT(floatOnTop(bool)));

    action = new QAction(tr("&Stop After This Video"), this);
    IconUtils::setIcon(action, "media-playback-stop");
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Escape));
    action->setCheckable(true);
    action->setEnabled(false);
    actionMap.insert("stopafterthis", action);
    connect(action, SIGNAL(toggled(bool)), SLOT(showStopAfterThisInStatusBar(bool)));

    action = new QAction(tr("&Report an Issue..."), this);
    actionMap.insert("reportIssue", action);
    connect(action, SIGNAL(triggered()), SLOT(reportIssue()));

    action = new QAction(tr("&Refine Search..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    action->setCheckable(true);
    action->setEnabled(false);
    actionMap.insert("refineSearch", action);

    action = new QAction(YTRegions::worldwideRegion().name, this);
    actionMap.insert("worldwideRegion", action);

    action = new QAction(YTRegions::localRegion().name, this);
    actionMap.insert("localRegion", action);

    action = new QAction(tr("More..."), this);
    actionMap.insert("moreRegion", action);

    action = new QAction(tr("&Related Videos"), this);
    IconUtils::setIcon(action, "view-list");
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    action->setStatusTip(tr("Watch videos related to the current one"));
    action->setEnabled(false);
    action->setPriority(QAction::LowPriority);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(relatedVideos()));
    actionMap.insert("relatedVideos", action);

    action = new QAction(tr("Open in &Browser..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    action->setEnabled(false);
    actionMap.insert("openInBrowser", action);
    connect(action, SIGNAL(triggered()), mediaView, SLOT(openInBrowser()));

    action = new QAction(tr("Restricted Mode"), this);
    IconUtils::setIcon(action, "safesearch");
    action->setStatusTip(tr("Hide videos that may contain inappropriate content"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_K));
    action->setCheckable(true);
    action->setVisible(VideoAPI::impl() != VideoAPI::IV);
    actionMap.insert("safeSearch", action);

    action = new QAction(tr("Toggle &Menu Bar"), this);
    connect(action, SIGNAL(triggered()), SLOT(toggleMenuVisibilityWithMessage()));
    actionMap.insert("toggleMenu", action);

    action = new QAction(tr("Menu"), this);
    IconUtils::setIcon(action, "open-menu");
    connect(action, SIGNAL(triggered()), SLOT(toggleToolbarMenu()));
    actionMap.insert("toolbarMenu", action);

#ifdef APP_MAC_STORE
    action = new QAction(tr("&Love %1? Rate it!").arg(Constants::NAME), this);
    actionMap.insert("appStore", action);
    connect(action, SIGNAL(triggered()), SLOT(rateOnAppStore()));
#endif

#ifdef APP_ACTIVATION
    ActivationView::createActivationAction(tr("Buy %1...").arg(Constants::NAME));
#endif

    // common action properties
    for (QAction *action : qAsConst(actionMap)) {
        // add actions to the MainWindow so that they work
        // when the menu is hidden
        addAction(action);
        setupAction(action);
    }
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&Application"));
#ifdef APP_ACTIVATION
    QAction *buyAction = getAction("buy");
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

    QMenu *playbackMenu = menuBar()->addMenu(tr("&Playback"));
    menuMap.insert("playback", playbackMenu);
    playbackMenu->addAction(pauseAct);
    playbackMenu->addAction(stopAct);
    playbackMenu->addAction(getAction("stopafterthis"));
    playbackMenu->addSeparator();
    playbackMenu->addAction(skipAct);
    playbackMenu->addAction(skipBackwardAct);
    playbackMenu->addSeparator();
    playbackMenu->addAction(getAction("manualplay"));
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
    playlistMenu->addAction(getAction("refineSearch"));

    QMenu *videoMenu = menuBar()->addMenu(tr("&Video"));
    menuMap.insert("video", videoMenu);
    videoMenu->addAction(getAction("relatedVideos"));
    videoMenu->addAction(findVideoPartsAct);
    videoMenu->addSeparator();
    videoMenu->addAction(getAction("subscribeChannel"));
#ifdef APP_SNAPSHOT
    videoMenu->addSeparator();
    videoMenu->addAction(getAction("snapshot"));
#endif
    videoMenu->addSeparator();
    videoMenu->addAction(webPageAct);
    videoMenu->addAction(copyLinkAct);
    videoMenu->addAction(getAction("openInBrowser"));
    videoMenu->addAction(getAction("download"));

    QMenu *shareMenu = menuBar()->addMenu(tr("&Share"));
    menuMap.insert("share", shareMenu);
    shareMenu->addAction(copyPageAct);
    shareMenu->addSeparator();
    shareMenu->addAction(getAction("twitter"));
    shareMenu->addAction(getAction("facebook"));
    shareMenu->addSeparator();
    shareMenu->addAction(getAction("email"));

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    menuMap.insert("view", viewMenu);
    viewMenu->addAction(getAction("ontop"));
    viewMenu->addAction(compactViewAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fullscreenAct);
#ifndef APP_MAC
    viewMenu->addSeparator();
    viewMenu->addAction(getAction("toggleMenu"));
#endif

#ifdef APP_MAC
    MacSupport::windowMenu(this);
#endif

    helpMenu = menuBar()->addMenu(tr("&Help"));
    menuMap.insert("help", helpMenu);
    helpMenu->addAction(siteAct);
#if !defined(APP_MAC) && !defined(APP_WIN)
    helpMenu->addAction(donateAct);
#endif
    helpMenu->addAction(getAction("reportIssue"));
    helpMenu->addAction(aboutAct);
#ifdef UPDATER
    helpMenu->addAction(Updater::instance().getAction());
#endif

#ifdef APP_MAC_STORE
    helpMenu->addSeparator();
    helpMenu->addAction(getAction("appStore"));
#endif
}

void MainWindow::createToolBar() {
    // Create widgets
    currentTimeLabel = new QLabel("00:00", this);

    seekSlider = new SeekSlider(this);
    seekSlider->setEnabled(false);
    seekSlider->setTracking(false);
    seekSlider->setMaximum(1000);
    volumeSlider = new SeekSlider(this);
    volumeSlider->setValue(volumeSlider->maximum());

#if defined(APP_MAC_SEARCHFIELD) && !defined(APP_MAC_QMACTOOLBAR)
    SearchWrapper *searchWrapper = new SearchWrapper(this);
    toolbarSearch = searchWrapper->getSearchLineEdit();
#else
    toolbarSearch = new SearchLineEdit(this);
#endif
    toolbarSearch->setMinimumWidth(toolbarSearch->fontInfo().pixelSize() * 15);
    toolbarSearch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolbarSearch->setSuggester(new YTSuggester(this));
    connect(toolbarSearch, SIGNAL(search(const QString &)), SLOT(search(const QString &)));
    connect(toolbarSearch, SIGNAL(suggestionAccepted(Suggestion *)),
            SLOT(suggestionAccepted(Suggestion *)));
    toolbarSearch->setStatusTip(searchFocusAct->statusTip());

    // Add widgets to toolbar

#ifdef APP_MAC_QMACTOOLBAR
    currentTimeLabel->hide();
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
#ifndef APP_LINUX
    mainToolBar->setIconSize(QSize(32, 32));
#endif
    mainToolBar->addAction(stopAct);
    QToolButton *stopToolButton =
            qobject_cast<QToolButton *>(mainToolBar->widgetForAction(stopAct));
    if (stopToolButton) {
        QMenu *stopMenu = new QMenu(this);
        stopMenu->addAction(getAction("stopafterthis"));
        stopToolButton->setMenu(stopMenu);
        stopToolButton->setPopupMode(QToolButton::DelayedPopup);
    }
    mainToolBar->addAction(pauseAct);
    mainToolBar->addAction(skipAct);
    mainToolBar->addAction(getAction("relatedVideos"));

    bool addFullScreenAct = true;
#ifdef Q_OS_MAC
    addFullScreenAct = !mac::CanGoFullScreen(winId());
#endif
    if (addFullScreenAct) mainToolBar->addAction(fullscreenAct);

    mainToolBar->addWidget(new Spacer());

    currentTimeLabel->setFont(FontUtils::small());
    currentTimeLabel->setMinimumWidth(currentTimeLabel->fontInfo().pixelSize() * 4);
    currentTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mainToolBar->addWidget(currentTimeLabel);

#ifdef APP_WIN
    mainToolBar->addWidget(new Spacer(nullptr, 10));
#endif

    seekSlider->setOrientation(Qt::Horizontal);
    seekSlider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    seekSlider->setFocusPolicy(Qt::NoFocus);
    mainToolBar->addWidget(seekSlider);

    mainToolBar->addWidget(new Spacer());

    mainToolBar->addAction(volumeMuteAct);
#ifndef APP_MAC_QMACTOOLBAR
    QToolButton *volumeMuteButton =
            qobject_cast<QToolButton *>(mainToolBar->widgetForAction(volumeMuteAct));
    volumeMuteButton->setIconSize(QSize(16, 16));
    auto fixVolumeMuteIconSize = [volumeMuteButton] {
        volumeMuteButton->setIcon(volumeMuteButton->icon().pixmap(16));
    };
    fixVolumeMuteIconSize();
    volumeMuteButton->connect(volumeMuteAct, &QAction::changed, volumeMuteButton,
                              fixVolumeMuteIconSize);
#endif

    volumeSlider->setStatusTip(
            tr("Press %1 to raise the volume, %2 to lower it")
                    .arg(volumeUpAct->shortcut().toString(QKeySequence::NativeText),
                         volumeDownAct->shortcut().toString(QKeySequence::NativeText)));

    volumeSlider->setOrientation(Qt::Horizontal);
    // this makes the volume slider smaller
    volumeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    volumeSlider->setFocusPolicy(Qt::NoFocus);
    mainToolBar->addWidget(volumeSlider);

    mainToolBar->addWidget(new Spacer());

#if defined(APP_MAC_SEARCHFIELD) && !defined(APP_MAC_QMACTOOLBAR)
    mainToolBar->addWidget(searchWrapper);
#else
    mainToolBar->addWidget(toolbarSearch);
    mainToolBar->addWidget(new Spacer(this, toolbarSearch->height() / 2));

    QAction *toolbarMenuAction = getAction("toolbarMenu");
    mainToolBar->addAction(toolbarMenuAction);
    toolbarMenuButton =
            qobject_cast<QToolButton *>(mainToolBar->widgetForAction(toolbarMenuAction));
#endif

    addToolBar(mainToolBar);
}

void MainWindow::createStatusBar() {
    statusToolBar = new QToolBar(statusBar());
    statusToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    statusToolBar->setIconSize(QSize(16, 16));

    regionAction = new QAction(this);
    regionAction->setStatusTip(tr("Choose your content location"));

    QAction *localAction = getAction("localRegion");
    if (!localAction->text().isEmpty()) {
        QMenu *regionMenu = new QMenu(this);
        regionMenu->addAction(getAction("worldwideRegion"));
        regionMenu->addAction(localAction);
        regionMenu->addSeparator();
        QAction *moreRegionsAction = getAction("moreRegion");
        regionMenu->addAction(moreRegionsAction);
        connect(moreRegionsAction, SIGNAL(triggered()), SLOT(showRegionsView()));
        regionAction->setMenu(regionMenu);
    }
    connect(regionAction, SIGNAL(triggered()), SLOT(showRegionsView()));

    /* Stupid code that generates the QRC items
    foreach(YTRegion r, YTRegions::list())
        qDebug() << QString("<file>flags/%1.png</file>").arg(r.id.toLower());
    */

    statusBar()->addPermanentWidget(statusToolBar);
    statusBar()->hide();
}

void MainWindow::showStopAfterThisInStatusBar(bool show) {
    QAction *action = getAction("stopafterthis");
    showActionsInStatusBar({action}, show);
}

void MainWindow::showActionsInStatusBar(const QVector<QAction *> &actions, bool show) {
#ifdef APP_EXTRA
    Extra::fadeInWidget(statusBar(), statusBar());
#endif
    for (auto action : actions) {
        if (show) {
            if (statusToolBar->actions().contains(action)) continue;
            if (statusToolBar->actions().isEmpty()) {
                statusToolBar->addAction(action);
            } else {
                statusToolBar->insertAction(statusToolBar->actions().at(0), action);
            }
        } else {
            statusToolBar->removeAction(action);
        }
    }

    if (show) {
        if (statusBar()->isHidden() && !fullScreenActive) setStatusBarVisibility(true);
    } else {
        if (statusBar()->isVisible() && !needStatusBar()) setStatusBarVisibility(false);
    }
}

void MainWindow::setStatusBarVisibility(bool show) {
    if (statusBar()->isVisible() != show) {
        statusBar()->setVisible(show);
        if (views->currentWidget() == mediaView)
            QTimer::singleShot(0, mediaView, SLOT(adjustWindowSize()));
    }
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
    QByteArray geometrySettings = settings.value("geometry").toByteArray();
    if (!geometrySettings.isEmpty()) {
        restoreGeometry(geometrySettings);
    } else {
        const QRect desktopSize = QGuiApplication::primaryScreen()->availableGeometry();
        int w = desktopSize.width() * .9;
        int h = qMin(w / 2, desktopSize.height());
        setGeometry(
                QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(w, h), desktopSize));
    }
    setDefinitionMode(settings.value("definition", YT3::instance().maxVideoDefinition().getName())
                              .toString());
    getAction("manualplay")->setChecked(settings.value("manualplay", false).toBool());
    getAction("safeSearch")->setChecked(settings.value("safeSearch", false).toBool());
#ifndef APP_MAC
    menuBar()->setVisible(settings.value("menuBar", false).toBool());
#endif
}

void MainWindow::writeSettings() {
    QSettings settings;

    if (!isReallyFullScreen()) {
        settings.setValue("geometry", saveGeometry());
        if (mediaView) mediaView->saveSplitterState();
    }

    settings.setValue("manualplay", getAction("manualplay")->isChecked());
    settings.setValue("safeSearch", getAction("safeSearch")->isChecked());
#ifndef APP_MAC
    settings.setValue("menuBar", menuBar()->isVisible());
#endif
}

void MainWindow::goBack() {
    if (history.size() > 1) {
        history.pop();
        showView(history.pop());
    }
}

void MainWindow::showView(View *view, bool transition) {
    if (!history.isEmpty() && view == history.top()) {
        qDebug() << "Attempting to show same view" << view;
        return;
    }

#ifdef APP_MAC
    if (transition && !history.isEmpty()) CompositeFader::go(this, this->grab());
#endif

    if (compactViewAct->isChecked()) compactViewAct->toggle();

    // call hide method on the current view
    View *oldView = qobject_cast<View *>(views->currentWidget());
    if (oldView) {
        oldView->disappear();
        oldView->setEnabled(false);
        oldView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    } else
        qDebug() << "Cannot cast old view";

    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    view->setEnabled(true);
    views->setCurrentWidget(view);
    view->appear();

    QString title = view->getTitle();
    if (title.isEmpty())
        title = Constants::NAME;
    else
        title += QLatin1String(" - ") + Constants::NAME;
    setWindowTitle(title);

    const bool isMediaView = view == mediaView;
    stopAct->setEnabled(isMediaView);
    compactViewAct->setEnabled(isMediaView);
    toolbarSearch->setEnabled(isMediaView);
    aboutAct->setEnabled(view != aboutView);
    getAction("downloads")->setChecked(view == downloadView);

    // dynamic view actions
    /* Not currently used by any view
    showActionsInStatusBar(viewActions, false);
    viewActions = newView->getViewActions();
    showActionsInStatusBar(viewActions, true);
    */

    history.push(view);
    emit viewChanged();
}

void MainWindow::about() {
    if (!aboutView) {
        aboutView = new AboutView(this);
        views->addWidget(aboutView);
    }
    showView(aboutView);
}

void MainWindow::visitSite() {
    QUrl url(Constants::WEBSITE);
    showMessage(QString(tr("Opening %1").arg(url.toString())));
    QDesktopServices::openUrl(url);
}

void MainWindow::donate() {
    QUrl url("https://" + QLatin1String(Constants::ORG_DOMAIN) + "/donate");
    showMessage(QString(tr("Opening %1").arg(url.toString())));
    QDesktopServices::openUrl(url);
}

void MainWindow::reportIssue() {
    QUrl url("https://flavio.tordini.org/forums/forum/minitube-forums/minitube-troubleshooting");
    QDesktopServices::openUrl(url);
}

void MainWindow::quit() {
#ifdef APP_MAC
    if (!confirmQuit()) {
        return;
    }
#endif
    // do not save geometry when in full screen or in compact mode
    if (!fullScreenActive && !compactViewAct->isChecked()) {
#ifdef APP_MAC
        hideToolbar();
#endif
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
        msgBox.setIconPixmap(IconUtils::pixmap(":/images/64x64/app.png", devicePixelRatioF()));
        msgBox.setText(
                tr("Do you want to exit %1 with a download in progress?").arg(Constants::NAME));
        msgBox.setInformativeText(
                tr("If you close %1 now, this download will be cancelled.").arg(Constants::NAME));
        msgBox.setModal(true);
        // make it a "sheet" on the Mac
        msgBox.setWindowModality(Qt::WindowModal);

        msgBox.addButton(tr("Close and cancel download"), QMessageBox::RejectRole);
        QPushButton *waitButton =
                msgBox.addButton(tr("Wait for download to finish"), QMessageBox::ActionRole);

        msgBox.exec();

        if (msgBox.clickedButton() == waitButton) {
            return false;
        }
    }
    return true;
}

void MainWindow::showHome() {
    showView(homeView);
    currentTimeLabel->clear();
    seekSlider->setValue(0);
}

void MainWindow::showMedia(SearchParams *searchParams) {
    showView(mediaView);
    if (getAction("safeSearch")->isChecked())
        searchParams->setSafeSearch(SearchParams::Strict);
    else
        searchParams->setSafeSearch(SearchParams::None);
    mediaView->search(searchParams);
}

void MainWindow::showMedia(VideoSource *videoSource) {
    showView(mediaView);
    mediaView->setVideoSource(videoSource);
}

void MainWindow::stateChanged(Media::State newState) {
    qDebug() << newState;

    seekSlider->setEnabled(newState != Media::StoppedState);

    switch (newState) {
    case Media::ErrorState:
        showMessage(tr("Error: %1").arg(media->errorString()));
        break;

    case Media::PlayingState:
        pauseAct->setEnabled(true);
        pauseAct->setIcon(IconUtils::icon("media-playback-pause"));
        pauseAct->setText(tr("&Pause"));
        pauseAct->setStatusTip(tr("Pause playback") + " (" +
                               pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        break;

    case Media::StoppedState:
        pauseAct->setEnabled(false);
        pauseAct->setIcon(IconUtils::icon("media-playback-start"));
        pauseAct->setText(tr("&Play"));
        pauseAct->setStatusTip(tr("Resume playback") + " (" +
                               pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        break;

    case Media::PausedState:
        pauseAct->setEnabled(true);
        pauseAct->setIcon(IconUtils::icon("media-playback-start"));
        pauseAct->setText(tr("&Play"));
        pauseAct->setStatusTip(tr("Resume playback") + " (" +
                               pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        break;

    case Media::BufferingState:
        pauseAct->setEnabled(false);
        pauseAct->setIcon(IconUtils::icon("content-loading"));
        pauseAct->setText(tr("&Loading..."));
        pauseAct->setStatusTip(QString());
        break;

    case Media::LoadingState:
        pauseAct->setEnabled(false);
        currentTimeLabel->clear();
        break;

    default:;
    }
}

void MainWindow::stop() {
    showHome();
    mediaView->stop();
}

void MainWindow::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);
#ifdef APP_MAC
    if (initialized && mac::CanGoFullScreen(winId())) {
        bool isFullscreen = mac::IsFullScreen(winId());
        if (isFullscreen != fullScreenActive) {
            if (compactViewAct->isChecked()) {
                compactViewAct->setChecked(false);
                compactView(false);
            }
            fullScreenActive = isFullscreen;
            updateUIForFullscreen();
        }
    }
#endif
#ifdef APP_MAC_QMACTOOLBAR
    int moreButtonWidth = 40;
    toolbarSearch->move(width() - toolbarSearch->width() - moreButtonWidth - 7, -34);
#endif
    hideMessage();
}

void MainWindow::enterEvent(QEvent *e) {
    Q_UNUSED(e);
#ifdef APP_MAC
    // Workaround cursor bug on macOS
    unsetCursor();
#endif
}

void MainWindow::leaveEvent(QEvent *e) {
    Q_UNUSED(e);
    if (fullScreenActive) hideFullscreenUI();
}

void MainWindow::toggleFullscreen() {
    if (compactViewAct->isChecked()) compactViewAct->toggle();

#ifdef APP_MAC
    WId handle = winId();
    if (mac::CanGoFullScreen(handle)) {
        if (mainToolBar) mainToolBar->setVisible(true);
        mac::ToggleFullScreen(handle);
        return;
    }
#endif

    fullScreenActive = !fullScreenActive;

    if (fullScreenActive) {
        // Enter full screen

        maximizedBeforeFullScreen = isMaximized();

        // save geometry now, if the user quits when in full screen
        // geometry won't be saved
        writeSettings();

#ifdef APP_MAC
        MacSupport::enterFullScreen(this, views);
#else
        menuVisibleBeforeFullScreen = menuBar()->isVisible();
        menuBar()->hide();
        if (mainToolBar) mainToolBar->hide();
        showFullScreen();
#endif

    } else {
        // Exit full screen

#ifdef APP_MAC
        MacSupport::exitFullScreen(this, views);
#else
        menuBar()->setVisible(menuVisibleBeforeFullScreen);
        if (mainToolBar) mainToolBar->setVisible(views->currentWidget() == mediaView);
        if (maximizedBeforeFullScreen)
            showMaximized();
        else
            showNormal();
#endif

        // Make sure the window has focus
        activateWindow();
    }

    qApp->processEvents();
    updateUIForFullscreen();
}

void MainWindow::updateUIForFullscreen() {
    static QList<QKeySequence> fsShortcuts;
    static QString fsText;

    if (fullScreenActive) {
        fsShortcuts = fullscreenAct->shortcuts();
        fsText = fullscreenAct->text();
        if (fsText.isEmpty()) qDebug() << "[taking Empty!]";
        fullscreenAct->setShortcuts(QList<QKeySequence>(fsShortcuts)
                                    << QKeySequence(Qt::Key_Escape));
        fullscreenAct->setText(tr("Leave &Full Screen"));
        fullscreenAct->setIcon(IconUtils::icon("view-restore"));
        setStatusBarVisibility(false);

        if (mainToolBar) {
            removeToolBar(mainToolBar);
            mainToolBar->move(0, 0);
        }

        mediaView->removeSidebar();

    } else {
        fullscreenAct->setShortcuts(fsShortcuts);
        if (fsText.isEmpty()) fsText = "[Empty!]";
        fullscreenAct->setText(fsText);
        fullscreenAct->setIcon(IconUtils::icon("view-fullscreen"));

        if (needStatusBar()) setStatusBarVisibility(true);

        if (mainToolBar) {
            addToolBar(mainToolBar);
        }

        mediaView->restoreSidebar();
    }

    // No compact view action when in full screen
    compactViewAct->setVisible(!fullScreenActive);
    compactViewAct->setChecked(false);

    // Hide anything but the video
    mediaView->setSidebarVisibility(!fullScreenActive);

    if (fullScreenActive) {
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_MediaStop));
    } else {
        stopAct->setShortcuts(QList<QKeySequence>()
                              << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    }

#ifdef Q_OS_MAC
    MacSupport::fullScreenActions(actionMap, fullScreenActive);
#endif

    if (views->currentWidget() == mediaView) mediaView->setFocus();

    if (fullScreenActive) {
        if (views->currentWidget() == mediaView) hideFullscreenUI();
    } else {
        fullscreenTimer->stop();
        unsetCursor();
    }
}

bool MainWindow::isReallyFullScreen() {
#ifdef Q_OS_MAC
    WId handle = winId();
    if (mac::CanGoFullScreen(handle))
        return mac::IsFullScreen(handle);
    else
        return isFullScreen();
#else
    return isFullScreen();
#endif
}

void MainWindow::missingKeyWarning() {
    static bool shown = false;
    if (shown) return;
    shown = true;
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(IconUtils::pixmap(":/images/64x64/app.png", devicePixelRatioF()));
    msgBox.setText(QString("%1 was built without a Google API key.").arg(Constants::NAME));
    msgBox.setInformativeText(QString("It won't work unless you enter one."
                                      "<p>In alternative you can get %1 from the developer site.")
                                      .arg(Constants::NAME));
    msgBox.setModal(true);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.addButton(QMessageBox::Close);
    QPushButton *enterKeyButton =
            msgBox.addButton(QString("Enter API key..."), QMessageBox::AcceptRole);
    QPushButton *devButton = msgBox.addButton(QString("Get from %1").arg(Constants::WEBSITE),
                                              QMessageBox::AcceptRole);
    QPushButton *helpButton = msgBox.addButton(QMessageBox::Help);

    msgBox.exec();

    if (msgBox.clickedButton() == helpButton) {
        QDesktopServices::openUrl(QUrl("https://github.com/flaviotordini/minitube/blob/master/"
                                       "README.md#google-api-key"));
    } else if (msgBox.clickedButton() == enterKeyButton) {
        bool ok;
        QString text = QInputDialog::getText(this, QString(), "Google API key:", QLineEdit::Normal,
                                             QString(), &ok);
        if (ok && !text.isEmpty()) {
            QSettings settings;
            settings.setValue("googleApiKey", text);
            YT3::instance().initApiKeys();
        }
    } else if (msgBox.clickedButton() == devButton) {
        QDesktopServices::openUrl(QUrl(Constants::WEBSITE));
    }
    shown = false;
}

void MainWindow::compactView(bool enable) {
    setUpdatesEnabled(false);

    compactModeActive = enable;

    static QList<QKeySequence> compactShortcuts;
    static QList<QKeySequence> stopShortcuts;

    const QString key = "compactGeometry";
    QSettings settings;

    if (enable) {
        setMinimumSize(320, 180);
#ifdef Q_OS_MAC
        mac::RemoveFullScreenWindow(winId());
#endif
        writeSettings();

        if (settings.contains(key))
            restoreGeometry(settings.value(key).toByteArray());
        else
            resize(480, 270);

#ifdef APP_MAC_QMACTOOLBAR
        mac::showToolBar(winId(), !enable);
#else
        mainToolBar->setVisible(!enable);
#endif
        mediaView->setSidebarVisibility(!enable);
        statusBar()->hide();

        compactShortcuts = compactViewAct->shortcuts();
        stopShortcuts = stopAct->shortcuts();

        QList<QKeySequence> newStopShortcuts(stopShortcuts);
        newStopShortcuts.removeAll(QKeySequence(Qt::Key_Escape));
        stopAct->setShortcuts(newStopShortcuts);
        compactViewAct->setShortcuts(QList<QKeySequence>(compactShortcuts)
                                     << QKeySequence(Qt::Key_Escape));

        // ensure focus does not end up to the search box
        // as it would steal the Space shortcut
        mediaView->setFocus();

    } else {
        settings.setValue(key, saveGeometry());

        // unset minimum size
        setMinimumSize(0, 0);

#ifdef Q_OS_MAC
        mac::SetupFullScreenWindow(winId());
#endif
#ifdef APP_MAC_QMACTOOLBAR
        mac::showToolBar(winId(), !enable);
#else
        mainToolBar->setVisible(!enable);
#endif
        mediaView->setSidebarVisibility(!enable);
        if (needStatusBar()) setStatusBarVisibility(true);

        readSettings();

        compactViewAct->setShortcuts(compactShortcuts);
        stopAct->setShortcuts(stopShortcuts);
    }

    // auto float on top
    floatOnTop(enable, false);

#ifdef APP_MAC
    mac::compactMode(winId(), enable);
#else
    if (enable) {
        menuVisibleBeforeCompactMode = menuBar()->isVisible();
        menuBar()->hide();
    } else {
        menuBar()->setVisible(menuVisibleBeforeCompactMode);
    }
#endif

    setUpdatesEnabled(true);
}

void MainWindow::toggleToolbarMenu() {
    if (!toolbarMenu) toolbarMenu = new ToolbarMenu(this);
    if (toolbarMenu->isVisible())
        toolbarMenu->hide();
    else
        toolbarMenu->show();
}

void MainWindow::searchFocus() {
    toolbarSearch->selectAll();
    toolbarSearch->setFocus();
}

void MainWindow::initMedia() {
#ifdef MEDIA_QTAV
    qFatal("QtAV has a showstopper bug. Audio stops randomly. See bug "
           "https://github.com/wang-bin/QtAV/issues/1184");
    media = new MediaQtAV(this);
#elif defined MEDIA_MPV
    media = new MediaMPV();
#else
    qFatal("No media backend defined");
#endif
    media->init();
    media->setUserAgent(HttpUtils::stealthUserAgent());

    QSettings settings;
    qreal volume = settings.value("volume", 1.).toReal();
    media->setVolume(volume);

    connect(media, &Media::error, this, &MainWindow::handleError);
    connect(media, &Media::stateChanged, this, &MainWindow::stateChanged);
    connect(media, &Media::positionChanged, this, &MainWindow::tick);

    connect(seekSlider, &QSlider::sliderMoved, this, [this](int value) {
        // value : maxValue = posit ion : duration
        qint64 ms = (value * media->duration()) / seekSlider->maximum();
        qDebug() << "Seeking to" << ms;
        media->seek(ms);
        if (media->state() == Media::PausedState) media->play();
    });
    connect(seekSlider, &QSlider::sliderPressed, this, [this]() {
        // value : maxValue = position : duration
        qint64 ms = (seekSlider->value() * media->duration()) / seekSlider->maximum();
        media->seek(ms);
        if (media->state() == Media::PausedState) media->play();
    });
    connect(media, &Media::started, this, [this]() { seekSlider->setValue(0); });

    connect(media, &Media::volumeChanged, this, &MainWindow::volumeChanged);
    connect(media, &Media::volumeMutedChanged, this, &MainWindow::volumeMutedChanged);
    connect(volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        qreal volume = (qreal)value / volumeSlider->maximum();
        media->setVolume(volume);
    });

    mediaView->setMedia(media);
}

void MainWindow::tick(qint64 time) {
#ifdef APP_MAC
    bool isDown = seekSlider->property("down").isValid();
#else
    bool isDown = seekSlider->isSliderDown();
#endif
    if (!isDown && media->state() == Media::PlayingState) {
        // value : maxValue = position : duration
        qint64 duration = media->duration();
        if (duration <= 0) return;
        int value = (seekSlider->maximum() * media->position()) / duration;
        seekSlider->setValue(value);
    }

    const QString s = formatTime(time);
    if (s != currentTimeLabel->text()) {
        currentTimeLabel->setText(s);
        emit currentTimeChanged(s);

        // remaining time
        const qint64 remainingTime = media->remainingTime();
        currentTimeLabel->setStatusTip(tr("Remaining time: %1").arg(formatTime(remainingTime)));
    }
}

QString MainWindow::formatTime(qint64 duration) {
    duration /= 1000;
    QString res;
    int seconds = (int)(duration % 60);
    duration /= 60;
    int minutes = (int)(duration % 60);
    duration /= 60;
    int hours = (int)(duration % 24);
    if (hours == 0) return res.sprintf("%02d:%02d", minutes, seconds);
    return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
}

void MainWindow::volumeUp() {
    qreal newVolume = media->volume() + .1;
    if (newVolume > 1.) newVolume = 1.;
    media->setVolume(newVolume);
}

void MainWindow::volumeDown() {
    qreal newVolume = media->volume() - .1;
    if (newVolume < 0) newVolume = 0;
    media->setVolume(newVolume);
}

void MainWindow::toggleVolumeMute() {
    bool muted = media->volumeMuted();
    media->setVolumeMuted(!muted);
}

void MainWindow::volumeChanged(qreal newVolume) {
    // automatically unmute when volume changes
    if (media->volumeMuted()) media->setVolumeMuted(false);
    showMessage(tr("Volume at %1%").arg((int)(newVolume * 100)));
    // newVolume : 1.0 = x : 1000
    int value = newVolume * volumeSlider->maximum();
    volumeSlider->blockSignals(true);
    volumeSlider->setValue(value);
    volumeSlider->blockSignals(false);
}

void MainWindow::volumeMutedChanged(bool muted) {
    if (muted) {
        volumeMuteAct->setIcon(IconUtils::icon("audio-volume-muted"));
        showMessage(tr("Volume is muted"));
    } else {
        volumeMuteAct->setIcon(IconUtils::icon("audio-volume-high"));
        showMessage(tr("Volume is unmuted"));
    }
}

void MainWindow::setDefinitionMode(const QString &definitionName) {
    QAction *definitionAct = getAction("definition");
    definitionAct->setText(definitionName);
    definitionAct->setStatusTip(
            tr("Maximum video definition set to %1").arg(definitionAct->text()) + " (" +
            definitionAct->shortcut().toString(QKeySequence::NativeText) + ")");
    showMessage(definitionAct->statusTip());
    YT3::instance().setMaxVideoDefinition(definitionName);
    if (views->currentWidget() == mediaView) {
        mediaView->reloadCurrentVideo();
    }
}

void MainWindow::toggleDefinitionMode() {
    const QVector<VideoDefinition> &definitions = VideoDefinition::getDefinitions();
    const VideoDefinition &currentDefinition = YT3::instance().maxVideoDefinition();

    int index = definitions.indexOf(currentDefinition);
    if (index != definitions.size() - 1) {
        index++;
    } else {
        index = 0;
    }
    setDefinitionMode(definitions.at(index).getName());
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
    if (views->currentWidget() == homeView &&
        homeView->currentWidget() == homeView->getSearchView())
        return;
    showActionsInStatusBar({getAction("manualplay")}, enabled);
}

void MainWindow::updateDownloadMessage(const QString &message) {
    getAction("downloads")->setText(message);
}

void MainWindow::downloadsFinished() {
    getAction("downloads")->setText(tr("&Downloads"));
    showMessage(tr("Downloads complete"));
}

void MainWindow::toggleDownloads(bool show) {
    if (show) {
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_MediaStop));
        getAction("downloads")
                ->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_J)
                                                     << QKeySequence(Qt::Key_Escape));
    } else {
        getAction("downloads")
                ->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_J));
        stopAct->setShortcuts(QList<QKeySequence>()
                              << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    }

    if (!downloadView) {
        downloadView = new DownloadView(this);
        views->addWidget(downloadView);
    }
    if (show)
        showView(downloadView);
    else
        goBack();
}

void MainWindow::suggestionAccepted(Suggestion *suggestion) {
    search(suggestion->value);
}

void MainWindow::search(const QString &query) {
    QString q = query.simplified();
    if (q.isEmpty()) return;
    SearchParams *searchParams = new SearchParams();
    searchParams->setKeywords(q);
    showMedia(searchParams);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = e->mimeData()->urls();
        if (urls.isEmpty()) return;
        const QUrl &url = urls.at(0);
        QString videoId = YTSearch::videoIdFromUrl(url.toString());
        if (!videoId.isEmpty()) e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e) {
    if (!toolbarSearch->isEnabled()) return;

    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.isEmpty()) return;
    const QUrl &url = urls.at(0);
    QString videoId = YTSearch::videoIdFromUrl(url.toString());
    if (!videoId.isEmpty()) {
        setWindowTitle(url.toString());
        SearchParams *searchParams = new SearchParams();
        searchParams->setKeywords(videoId);
        showMedia(searchParams);
    }
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
    if (showAction) showActionsInStatusBar({getAction("ontop")}, onTop);
#ifdef APP_MAC
    mac::floatOnTop(winId(), onTop);
#else
    if (onTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        show();
    } else {
        setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
        show();
    }
#endif
}

void MainWindow::restore() {
#ifdef APP_MAC
    mac::uncloseWindow(winId());
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
        getAction("stopafterthis")->toggle();
    } else if (message.startsWith("--")) {
        MainWindow::printHelp();
    } else if (!message.isEmpty()) {
        SearchParams *searchParams = new SearchParams();
        searchParams->setKeywords(message);
        showMedia(searchParams);
    }
}

void MainWindow::hideFullscreenUI() {
    if (views->currentWidget() != mediaView) return;
    setCursor(Qt::BlankCursor);

    QPoint p = mapFromGlobal(QCursor::pos());
    const int x = p.x();

    if (x > mediaView->getSidebar()->width()) mediaView->setSidebarVisibility(false);

#ifndef APP_MAC
    const int y = p.y();
    bool shouldHideToolbar = !toolbarSearch->hasFocus() && y > mainToolBar->height();
    if (shouldHideToolbar) mainToolBar->setVisible(false);
#endif
}

void MainWindow::toggleMenuVisibility() {
    bool show = !menuBar()->isVisible();
    menuBar()->setVisible(show);
}

void MainWindow::toggleMenuVisibilityWithMessage() {
    bool show = !menuBar()->isVisible();
    menuBar()->setVisible(show);
    if (!show) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("You can still access the menu bar by pressing the ALT key"));
        msgBox.setModal(true);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.exec();
    }
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

void MainWindow::setupAction(QAction *action) {
    // never autorepeat.
    // unexperienced users tend to keep keys pressed for a "long" time
    action->setAutoRepeat(false);

    // show keyboard shortcuts in the status bar
    if (!action->shortcut().isEmpty())
        action->setStatusTip(action->statusTip() + QLatin1String(" (") +
                             action->shortcut().toString(QKeySequence::NativeText) +
                             QLatin1String(")"));
}

QAction *MainWindow::getAction(const char *name) {
    return actionMap.value(QByteArray::fromRawData(name, strlen(name)));
}

void MainWindow::addNamedAction(const QByteArray &name, QAction *action) {
    actionMap.insert(name, action);
}

QMenu *MainWindow::getMenu(const char *name) {
    return menuMap.value(QByteArray::fromRawData(name, strlen(name)));
}

void MainWindow::showMessage(const QString &message) {
    if (!isVisible()) return;
#ifdef APP_MAC
    if (!mac::isVisible(winId())) return;
#endif
    if (statusBar()->isVisible())
        statusBar()->showMessage(message, 60000);
    else if (isActiveWindow()) {
        messageLabel->setText(message);
        QSize size = messageLabel->sizeHint();
        // round width to avoid flicker with fast changing messages (e.g. volume
        // changes)
        int w = size.width() + 10;
        const int multiple = 15;
        w = w + multiple / 2;
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
    if (messageLabel->isVisible()) {
        messageLabel->hide();
        messageLabel->clear();
    }
}

void MainWindow::handleError(const QString &message) {
    qWarning() << message;
    showMessage(message);
}

#ifdef APP_ACTIVATION
void MainWindow::showActivationView() {
    View *activationView = ActivationView::instance();
    views->addWidget(activationView);
    if (views->currentWidget() != activationView) showView(activationView);
}
#endif

void MainWindow::showRegionsView() {
    if (!regionsView) {
        regionsView = new RegionsView(this);
        connect(regionsView, SIGNAL(regionChanged()), homeView->getStandardFeedsView(),
                SLOT(load()));
        views->addWidget(regionsView);
    }
    showView(regionsView);
}
