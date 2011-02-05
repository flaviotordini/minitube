#include "MainWindow.h"
#include "spacer.h"
#include "constants.h"
#include "iconloader/qticonloader.h"
#include "global.h"
#include "videodefinition.h"
#include "fontutils.h"
#include "globalshortcuts.h"
#ifdef Q_WS_X11
#include "gnomeglobalshortcutbackend.h"
#endif
#ifdef APP_MAC
// #include "local/mac/mac_startup.h"
#endif
#include "downloadmanager.h"
#include "youtubesuggest.h"

MainWindow::MainWindow() :
        aboutView(0),
        downloadView(0),
        mediaObject(0),
        audioOutput(0),
        m_fullscreen(false) {

    // views mechanism
    history = new QStack<QWidget*>();
    views = new QStackedWidget(this);
    setCentralWidget(views);

    // views
    searchView = new SearchView(this);
    connect(searchView, SIGNAL(search(SearchParams*)), this, SLOT(showMedia(SearchParams*)));
    views->addWidget(searchView);

    mediaView = new MediaView(this);
    views->addWidget(mediaView);

    toolbarSearch = new SearchLineEdit(this);
    toolbarSearch->setMinimumWidth(toolbarSearch->fontInfo().pixelSize()*15);
    toolbarSearch->setSuggester(new YouTubeSuggest(this));
    connect(toolbarSearch, SIGNAL(search(const QString&)), this, SLOT(startToolbarSearch(const QString&)));

    // build ui
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    initPhonon();
    // mediaView->setSlider(slider);
    mediaView->setMediaObject(mediaObject);

    // remove that useless menu/toolbar context menu
    this->setContextMenuPolicy(Qt::NoContextMenu);

    // mediaView init stuff thats needs actions
    mediaView->initialize();

    // event filter to block ugly toolbar tooltips
    qApp->installEventFilter(this);

    // restore window position
    readSettings();

    // show the initial view
    showWidget(searchView);

    // Global shortcuts
    GlobalShortcuts &shortcuts = GlobalShortcuts::instance();
#ifdef Q_WS_X11
    if (GnomeGlobalShortcutBackend::IsGsdAvailable())
        shortcuts.setBackend(new GnomeGlobalShortcutBackend(&shortcuts));
#endif
#ifdef APP_MAC
    // mac::MacSetup();
#endif
    connect(&shortcuts, SIGNAL(PlayPause()), pauseAct, SLOT(trigger()));
    connect(&shortcuts, SIGNAL(Stop()), this, SLOT(stop()));
    connect(&shortcuts, SIGNAL(Next()), skipAct, SLOT(trigger()));

    connect(DownloadManager::instance(), SIGNAL(statusMessageChanged(QString)),
            SLOT(updateDownloadMessage(QString)));
    connect(DownloadManager::instance(), SIGNAL(finished()),
            SLOT(downloadsFinished()));
}

MainWindow::~MainWindow() {
    delete history;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove && this->m_fullscreen) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        int x = mouseEvent->pos().x();
        int y = mouseEvent->pos().y();

        if (y < 0 && (obj == this->mainToolBar || !(y <= 10-this->mainToolBar->height() && y >= 0-this->mainToolBar->height() )))
           this->mainToolBar->setVisible(false);
        if (x < 0)
            this->mediaView->setPlaylistVisible(false);
    }

    if (event->type() == QEvent::ToolTip) {
        // kill tooltips
        return true;
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

void MainWindow::createActions() {

    QMap<QString, QAction*> *actions = The::globalActions();

    stopAct = new QAction(QtIconLoader::icon("media-playback-stop"), tr("&Stop"), this);
    stopAct->setStatusTip(tr("Stop playback and go back to the search view"));
    stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    stopAct->setEnabled(false);
    actions->insert("stop", stopAct);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(stop()));

    skipAct = new QAction(QtIconLoader::icon("media-skip-forward"), tr("S&kip"), this);
    skipAct->setStatusTip(tr("Skip to the next video"));
    skipAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Right) << QKeySequence(Qt::Key_MediaNext));
    skipAct->setEnabled(false);
    actions->insert("skip", skipAct);
    connect(skipAct, SIGNAL(triggered()), mediaView, SLOT(skip()));

    pauseAct = new QAction(QtIconLoader::icon("media-playback-pause"), tr("&Pause"), this);
    pauseAct->setStatusTip(tr("Pause playback"));
    pauseAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Space) << QKeySequence(Qt::Key_MediaPlay));
    pauseAct->setEnabled(false);
    actions->insert("pause", pauseAct);
    connect(pauseAct, SIGNAL(triggered()), mediaView, SLOT(pause()));

    fullscreenAct = new QAction(QtIconLoader::icon("view-fullscreen"), tr("&Full Screen"), this);
    fullscreenAct->setStatusTip(tr("Go full screen"));
    fullscreenAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
    fullscreenAct->setShortcutContext(Qt::ApplicationShortcut);
#if QT_VERSION >= 0x040600
    fullscreenAct->setPriority(QAction::LowPriority);
#endif
    actions->insert("fullscreen", fullscreenAct);
    connect(fullscreenAct, SIGNAL(triggered()), this, SLOT(fullscreen()));

    compactViewAct = new QAction(tr("&Compact mode"), this);
    compactViewAct->setStatusTip(tr("Hide the playlist and the toolbar"));
    compactViewAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
    compactViewAct->setCheckable(true);
    compactViewAct->setChecked(false);
    compactViewAct->setEnabled(false);
    actions->insert("compactView", compactViewAct);
    connect(compactViewAct, SIGNAL(toggled(bool)), this, SLOT(compactView(bool)));

    webPageAct = new QAction(tr("Open the &YouTube page"), this);
    webPageAct->setStatusTip(tr("Go to the YouTube video page and pause playback"));
    webPageAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
    webPageAct->setEnabled(false);
    actions->insert("webpage", webPageAct);
    connect(webPageAct, SIGNAL(triggered()), mediaView, SLOT(openWebPage()));

    copyPageAct = new QAction(tr("Copy the YouTube &link"), this);
    copyPageAct->setStatusTip(tr("Copy the current video YouTube link to the clipboard"));
    copyPageAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    copyPageAct->setEnabled(false);
    actions->insert("pagelink", copyPageAct);
    connect(copyPageAct, SIGNAL(triggered()), mediaView, SLOT(copyWebPage()));

    copyLinkAct = new QAction(tr("Copy the video stream &URL"), this);
    copyLinkAct->setStatusTip(tr("Copy the current video stream URL to the clipboard"));
    copyLinkAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    copyLinkAct->setEnabled(false);
    actions->insert("videolink", copyLinkAct);
    connect(copyLinkAct, SIGNAL(triggered()), mediaView, SLOT(copyVideoLink()));

    removeAct = new QAction(tr("&Remove"), this);
    removeAct->setStatusTip(tr("Remove the selected videos from the playlist"));
    removeAct->setShortcuts(QList<QKeySequence>() << QKeySequence("Del") << QKeySequence("Backspace"));
    removeAct->setEnabled(false);
    actions->insert("remove", removeAct);
    connect(removeAct, SIGNAL(triggered()), mediaView, SLOT(removeSelected()));

    moveUpAct = new QAction(tr("Move &Up"), this);
    moveUpAct->setStatusTip(tr("Move up the selected videos in the playlist"));
    moveUpAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up));
    moveUpAct->setEnabled(false);
    actions->insert("moveUp", moveUpAct);
    connect(moveUpAct, SIGNAL(triggered()), mediaView, SLOT(moveUpSelected()));

    moveDownAct = new QAction(tr("Move &Down"), this);
    moveDownAct->setStatusTip(tr("Move down the selected videos in the playlist"));
    moveDownAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down));
    moveDownAct->setEnabled(false);
    actions->insert("moveDown", moveDownAct);
    connect(moveDownAct, SIGNAL(triggered()), mediaView, SLOT(moveDownSelected()));

    clearAct = new QAction(tr("&Clear recent keywords"), this);
    clearAct->setMenuRole(QAction::ApplicationSpecificRole);
    clearAct->setShortcuts(QList<QKeySequence>()
                           << QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete)
                           << QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Backspace));
    clearAct->setStatusTip(tr("Clear the search history. Cannot be undone."));
    clearAct->setEnabled(true);
    actions->insert("clearRecentKeywords", clearAct);
    connect(clearAct, SIGNAL(triggered()), SLOT(clearRecentKeywords()));

    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setMenuRole(QAction::QuitRole);
    quitAct->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Q")) << QKeySequence(Qt::CTRL + Qt::Key_W));
    quitAct->setStatusTip(tr("Bye"));
    actions->insert("quit", quitAct);
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    siteAct = new QAction(tr("&Website"), this);
    siteAct->setShortcut(QKeySequence::HelpContents);
    siteAct->setStatusTip(tr("%1 on the Web").arg(Constants::APP_NAME));
    actions->insert("site", siteAct);
    connect(siteAct, SIGNAL(triggered()), this, SLOT(visitSite()));

    donateAct = new QAction(tr("Make a &donation"), this);
    donateAct->setStatusTip(tr("Please support the continued development of %1").arg(Constants::APP_NAME));
    actions->insert("donate", donateAct);
    connect(donateAct, SIGNAL(triggered()), this, SLOT(donate()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setMenuRole(QAction::AboutRole);
    aboutAct->setStatusTip(tr("Info about %1").arg(Constants::APP_NAME));
    actions->insert("about", aboutAct);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    // Invisible actions

    searchFocusAct = new QAction(this);
    searchFocusAct->setShortcut(QKeySequence::Find);
    searchFocusAct->setStatusTip(tr("Search"));
    actions->insert("search", searchFocusAct);
    connect(searchFocusAct, SIGNAL(triggered()), this, SLOT(searchFocus()));
    addAction(searchFocusAct);

    volumeUpAct = new QAction(this);
    volumeUpAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Plus) << QKeySequence(Qt::Key_VolumeUp));
    actions->insert("volume-up", volumeUpAct);
    connect(volumeUpAct, SIGNAL(triggered()), this, SLOT(volumeUp()));
    addAction(volumeUpAct);

    volumeDownAct = new QAction(this);
    volumeDownAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Minus) << QKeySequence(Qt::Key_VolumeDown));
    actions->insert("volume-down", volumeDownAct);
    connect(volumeDownAct, SIGNAL(triggered()), this, SLOT(volumeDown()));
    addAction(volumeDownAct);

    volumeMuteAct = new QAction(this);
    volumeMuteAct->setIcon(QtIconLoader::icon("audio-volume-high"));
    volumeMuteAct->setStatusTip(tr("Mute volume"));
    volumeMuteAct->setShortcuts(QList<QKeySequence>()
                                << QKeySequence(tr("Ctrl+M"))
                                << QKeySequence(Qt::Key_VolumeMute));
    actions->insert("volume-mute", volumeMuteAct);
    connect(volumeMuteAct, SIGNAL(triggered()), SLOT(volumeMute()));
    addAction(volumeMuteAct);

    QAction *definitionAct = new QAction(this);
    definitionAct->setIcon(QtIconLoader::icon("video-display"));
    definitionAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_D));
    /*
    QMenu *definitionMenu = new QMenu(this);
    foreach (QString definition, VideoDefinition::getDefinitionNames()) {
        definitionMenu->addAction(definition);
    }
    definitionAct->setMenu(definitionMenu);
    */
    actions->insert("definition", definitionAct);
    connect(definitionAct, SIGNAL(triggered()), SLOT(toggleDefinitionMode()));
    addAction(definitionAct);

    QAction *action;

    /*
    action = new QAction(tr("&Autoplay"), this);
    action->setStatusTip(tr("Automatically start playing videos"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), SLOT(setAutoplay(bool)));
    actions->insert("autoplay", action);
    */

    action = new QAction(tr("&Downloads"), this);
    action->setStatusTip(tr("Show details about video downloads"));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    action->setCheckable(true);
    action->setIcon(QtIconLoader::icon("go-down"));
    action->setVisible(false);
    connect(action, SIGNAL(toggled(bool)), SLOT(toggleDownloads(bool)));
    actions->insert("downloads", action);

    action = new QAction(tr("&Download"), this);
    action->setStatusTip(tr("Download the current video"));
    action->setShortcut(QKeySequence::Save);
    action->setIcon(QtIconLoader::icon("go-down"));
    action->setEnabled(false);
#if QT_VERSION >= 0x040600
    action->setPriority(QAction::LowPriority);
#endif
    connect(action, SIGNAL(triggered()), mediaView, SLOT(downloadVideo()));
    actions->insert("download", action);

    // common action properties
    foreach (QAction *action, actions->values()) {

        // add actions to the MainWindow so that they work
        // when the menu is hidden
        addAction(action);

        // never autorepeat.
        // unexperienced users tend to keep keys pressed for a "long" time
        action->setAutoRepeat(false);

        // set to something more meaningful then the toolbar text
        // HELP! how to remove tooltips altogether?!
        if (!action->statusTip().isEmpty())
            action->setToolTip(action->statusTip());

        // show keyboard shortcuts in the status bar
        if (!action->shortcut().isEmpty())
            action->setStatusTip(action->statusTip() + " (" + action->shortcut().toString(QKeySequence::NativeText) + ")");

        // no icons in menus
        action->setIconVisibleInMenu(false);

    }

}

void MainWindow::createMenus() {

    QMap<QString, QMenu*> *menus = The::globalMenus();

    fileMenu = menuBar()->addMenu(tr("&Application"));
    // menus->insert("file", fileMenu);
    fileMenu->addAction(clearAct);
#ifndef APP_MAC
    fileMenu->addSeparator();
#endif
    fileMenu->addAction(quitAct);

    playlistMenu = menuBar()->addMenu(tr("&Playlist"));
    menus->insert("playlist", playlistMenu);
    playlistMenu->addAction(removeAct);
    playlistMenu->addSeparator();
    playlistMenu->addAction(moveUpAct);
    playlistMenu->addAction(moveDownAct);

    viewMenu = menuBar()->addMenu(tr("&Video"));
    menus->insert("video", viewMenu);
    viewMenu->addAction(stopAct);
    viewMenu->addAction(pauseAct);
    viewMenu->addAction(skipAct);
    viewMenu->addSeparator();
    viewMenu->addAction(The::globalActions()->value("download"));
    viewMenu->addSeparator();
    viewMenu->addAction(webPageAct);
    viewMenu->addAction(copyPageAct);
    viewMenu->addAction(copyLinkAct);
    viewMenu->addSeparator();
    viewMenu->addAction(compactViewAct);
    viewMenu->addAction(fullscreenAct);
#ifdef APP_MAC
    extern void qt_mac_set_dock_menu(QMenu *);
    qt_mac_set_dock_menu(viewMenu);
#endif

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(siteAct);
#if !defined(APP_MAC) && !defined(APP_WIN)
    helpMenu->addAction(donateAct);
#endif
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars() {

    setUnifiedTitleAndToolBarOnMac(true);

    mainToolBar = new QToolBar(this);
#if QT_VERSION < 0x040600 | defined(APP_MAC)
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
#elif defined(APP_WIN)
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->setStyleSheet(
            "QToolBar {"
                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fafcfd, stop:.5 #e6f0fa, stop:.51 #dde9f7, stop:1 #dde9f7);"
                "border: 0;"
                "border-bottom: 1px solid #a0afc3;"
            "}");
#else
    mainToolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
#endif
    mainToolBar->setFloatable(false);
    mainToolBar->setMovable(false);

#if defined(APP_MAC) | defined(APP_WIN)
    mainToolBar->setIconSize(QSize(32, 32));
#endif

    mainToolBar->addAction(stopAct);
    mainToolBar->addAction(pauseAct);
    mainToolBar->addAction(skipAct);
    mainToolBar->addAction(fullscreenAct);
    mainToolBar->addAction(The::globalActions()->value("download"));

    mainToolBar->addWidget(new Spacer());

    QFont smallerFont = FontUtils::small();
    currentTime = new QLabel(mainToolBar);
    currentTime->setFont(smallerFont);
    mainToolBar->addWidget(currentTime);

    mainToolBar->addWidget(new Spacer());

    seekSlider = new Phonon::SeekSlider(this);
    seekSlider->setIconVisible(false);
    seekSlider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    mainToolBar->addWidget(seekSlider);

/*
    mainToolBar->addWidget(new Spacer());
    slider = new QSlider(this);
    slider->setOrientation(Qt::Horizontal);
    slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    mainToolBar->addWidget(slider);
*/

    mainToolBar->addWidget(new Spacer());

    totalTime = new QLabel(mainToolBar);
    totalTime->setFont(smallerFont);
    mainToolBar->addWidget(totalTime);

    mainToolBar->addWidget(new Spacer());

    mainToolBar->addAction(volumeMuteAct);

    volumeSlider = new Phonon::VolumeSlider(this);
    volumeSlider->setMuteVisible(false);
    // qDebug() << volumeSlider->children();
    // status tip for the volume slider
    QSlider* volumeQSlider = volumeSlider->findChild<QSlider*>();
    if (volumeQSlider)
        volumeQSlider->setStatusTip(tr("Press %1 to raise the volume, %2 to lower it").arg(
                volumeUpAct->shortcut().toString(QKeySequence::NativeText), volumeDownAct->shortcut().toString(QKeySequence::NativeText)));
    // this makes the volume slider smaller
    volumeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainToolBar->addWidget(volumeSlider);

    mainToolBar->addWidget(new Spacer());

    toolbarSearch->setStatusTip(searchFocusAct->statusTip());
    mainToolBar->addWidget(toolbarSearch);

    mainToolBar->addWidget(new Spacer());

    addToolBar(mainToolBar);
}

void MainWindow::createStatusBar() {

    // remove ugly borders on OSX
    // also remove excessive spacing
    statusBar()->setStyleSheet("::item{border:0 solid} QToolBar {padding:0;spacing:0;margin:0;border:0}");

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setIconSize(QSize(16, 16));
    toolBar->addAction(The::globalActions()->value("downloads"));
    // toolBar->addAction(The::globalActions()->value("autoplay"));
    toolBar->addAction(The::globalActions()->value("definition"));
    statusBar()->addPermanentWidget(toolBar);

    statusBar()->show();
}

void MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
#ifdef APP_MAC
    if (!isMaximized())
        move(x(), y() + mainToolBar->height() + 8);
#endif
    setDefinitionMode(settings.value("definition", VideoDefinition::getDefinitionNames().first()).toString());
    audioOutput->setVolume(settings.value("volume", 1).toDouble());
    audioOutput->setMuted(settings.value("volumeMute").toBool());
}

void MainWindow::writeSettings() {

    QSettings settings;

    // do not save geometry when in full screen
    if (!m_fullscreen) {
        settings.setValue("geometry", saveGeometry());
    }

    settings.setValue("volume", audioOutput->volume());
    settings.setValue("volumeMute", audioOutput->isMuted());
    mediaView->saveSplitterState();
}

void MainWindow::goBack() {
    if ( history->size() > 1 ) {
        history->pop();
        QWidget *widget = history->pop();
        showWidget(widget);
    }
}

void MainWindow::showWidget ( QWidget* widget ) {

    setUpdatesEnabled(false);

    // call hide method on the current view
    View* oldView = dynamic_cast<View *> (views->currentWidget());
    if (oldView) {
        oldView->disappear();
    }

    // call show method on the new view
    View* newView = dynamic_cast<View *> (widget);
    if (newView) {
        newView->appear();
        QMap<QString,QVariant> metadata = newView->metadata();
        QString windowTitle = metadata.value("title").toString();
        if (windowTitle.length())
            windowTitle += " - ";
        setWindowTitle(windowTitle + Constants::APP_NAME);
        statusBar()->showMessage((metadata.value("description").toString()));
    }

    stopAct->setEnabled(widget == mediaView);
    fullscreenAct->setEnabled(widget == mediaView);
    compactViewAct->setEnabled(widget == mediaView);
    webPageAct->setEnabled(widget == mediaView);
    copyPageAct->setEnabled(widget == mediaView);
    copyLinkAct->setEnabled(widget == mediaView);
    aboutAct->setEnabled(widget != aboutView);
    The::globalActions()->value("download")->setEnabled(widget == mediaView);
    The::globalActions()->value("downloads")->setChecked(widget == downloadView);

    // toolbar only for the mediaView
    /* mainToolBar->setVisible(
            (widget == mediaView && !compactViewAct->isChecked())
            || widget == downloadView
            ); */

    setUpdatesEnabled(true);

    QWidget *oldWidget = views->currentWidget();
    views->setCurrentWidget(widget);

#ifdef APP_MAC
    // crossfade only on OSX
    // where we can be sure of video performance
    fadeInWidget(oldWidget, widget);
#endif

    history->push(widget);
}

void MainWindow::fadeInWidget(QWidget *oldWidget, QWidget *newWidget) {
    if (faderWidget) faderWidget->close();
    if (!oldWidget || !newWidget) {
        // qDebug() << "no widgets";
        return;
    }
    faderWidget = new FaderWidget(newWidget);
    faderWidget->start(QPixmap::grabWidget(oldWidget));
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
    statusBar()->showMessage(QString(tr("Opening %1").arg(url.toString())));
    QDesktopServices::openUrl(url);
}

void MainWindow::donate() {
    QUrl url(QString(Constants::WEBSITE) + "#donate");
    statusBar()->showMessage(QString(tr("Opening %1").arg(url.toString())));
    QDesktopServices::openUrl(url);
}

void MainWindow::quit() {
    writeSettings();
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (DownloadManager::instance()->activeItems() > 0) {
        QMessageBox msgBox;
        msgBox.setIconPixmap(QPixmap(":/images/app.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setText(tr("Do you want to exit %1 with a download in progress?").arg(Constants::APP_NAME));
        msgBox.setInformativeText(tr("If you close %1 now, this download will be cancelled.").arg(Constants::APP_NAME));
        msgBox.setModal(true);

        msgBox.addButton(tr("Close and cancel download"), QMessageBox::RejectRole);
        QPushButton *waitButton = msgBox.addButton(tr("Wait for download to finish"), QMessageBox::ActionRole);

        msgBox.exec();

        if (msgBox.clickedButton() == waitButton) {
            event->ignore();
            return;
        }

    }
    quit();
    QWidget::closeEvent(event);
}

void MainWindow::showSearch() {
    showWidget(searchView);
    currentTime->clear();
    totalTime->clear();
}

void MainWindow::showMedia(SearchParams *searchParams) {
    mediaView->search(searchParams);
    showWidget(mediaView);
}

void MainWindow::stateChanged(Phonon::State newState, Phonon::State /* oldState */) {

    // qDebug() << "Phonon state: " << newState;

    switch (newState) {

    case Phonon::ErrorState:
        if (mediaObject->errorType() == Phonon::FatalError) {
            statusBar()->showMessage(tr("Fatal error: %1").arg(mediaObject->errorString()));
        } else {
            statusBar()->showMessage(tr("Error: %1").arg(mediaObject->errorString()));
        }
        break;

         case Phonon::PlayingState:
        pauseAct->setEnabled(true);
        pauseAct->setIcon(QtIconLoader::icon("media-playback-pause"));
        pauseAct->setText(tr("&Pause"));
        pauseAct->setStatusTip(tr("Pause playback") + " (" +  pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        skipAct->setEnabled(true);
        // stopAct->setEnabled(true);
        break;

         case Phonon::StoppedState:
        pauseAct->setEnabled(false);
        skipAct->setEnabled(false);
        // stopAct->setEnabled(false);
        break;

         case Phonon::PausedState:
        skipAct->setEnabled(true);
        pauseAct->setEnabled(true);
        pauseAct->setIcon(QtIconLoader::icon("media-playback-start"));
        pauseAct->setText(tr("&Play"));
        pauseAct->setStatusTip(tr("Resume playback") + " (" +  pauseAct->shortcut().toString(QKeySequence::NativeText) + ")");
        // stopAct->setEnabled(true);
        break;

         case Phonon::BufferingState:
         case Phonon::LoadingState:
        skipAct->setEnabled(true);
        pauseAct->setEnabled(false);
        currentTime->clear();
        totalTime->clear();
        // stopAct->setEnabled(true);
        break;

         default:
        ;
    }
}

void MainWindow::stop() {
    mediaView->stop();
    showSearch();
}

void MainWindow::fullscreen() {

    // No compact view action when in full screen
    compactViewAct->setVisible(m_fullscreen);
    compactViewAct->setChecked(false);

    // Also no Youtube action since it opens a new window
    webPageAct->setVisible(m_fullscreen);
    copyPageAct->setVisible(m_fullscreen);
    copyLinkAct->setVisible(m_fullscreen);

    stopAct->setVisible(m_fullscreen);

    // workaround: prevent focus on the search bar
    // it steals the Space key needed for Play/Pause
    toolbarSearch->setEnabled(m_fullscreen);

    // Hide anything but the video
    mediaView->setPlaylistVisible(m_fullscreen);
    statusBar()->setVisible(m_fullscreen);

#ifndef APP_MAC
    menuBar()->setVisible(m_fullscreen);
#endif

#ifdef APP_MAC
    // make the actions work when video is fullscreen (on the Mac)
    QMap<QString, QAction*> *actions = The::globalActions();
    foreach (QAction *action, actions->values()) {
        if (m_fullscreen) {
            action->setShortcutContext(Qt::WindowShortcut);
        } else {
            action->setShortcutContext(Qt::ApplicationShortcut);
        }
    }
#endif

    if (m_fullscreen) {

        // Exit full screen

        // use setShortcuts instead of setShortcut
        // the latter seems not to work
        fullscreenAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::ALT + Qt::Key_Return));
        fullscreenAct->setText(tr("&Full Screen"));
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));

#ifdef APP_MAC
        setCentralWidget(views);
        views->showNormal();
        show();
        mediaView->setFocus();
#else
        mainToolBar->show();
        if (m_maximized) showMaximized();
        else showNormal();
#endif

        // Make sure the window has focus
        activateWindow();

    } else {

        // Enter full screen

        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_MediaStop));
        fullscreenAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::ALT + Qt::Key_Return));
        fullscreenAct->setText(tr("Exit &Full Screen"));
        m_maximized = isMaximized();

        // save geometry now, if the user quits when in full screen
        // geometry won't be saved
        writeSettings();

#ifdef APP_MAC
        hide();
        views->setParent(0);
        QTimer::singleShot(0, views, SLOT(showFullScreen()));
#else
        mainToolBar->hide();
        showFullScreen();
#endif

    }

    m_fullscreen = !m_fullscreen;

}

void MainWindow::compactView(bool enable) {

    mediaView->setPlaylistVisible(!enable);
    statusBar()->setVisible(!enable);

#ifndef APP_MAC
    menuBar()->setVisible(!enable);
#endif

    if (enable) {
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_MediaStop));
        compactViewAct->setShortcuts(
                QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_Return)
                << QKeySequence(Qt::Key_Escape));

        // ensure focus does not end up to the search box
        // as it would steal the Space shortcut
        mediaView->setFocus();
    } else {
        compactViewAct->setShortcuts(QList<QKeySequence>() <<  QKeySequence(Qt::CTRL + Qt::Key_Return));
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::Key_MediaStop));
    }

}

void MainWindow::searchFocus() {
    QWidget *view = views->currentWidget();
    if (view == mediaView) {
        toolbarSearch->selectAll();
        toolbarSearch->setFocus();
    }
}

void MainWindow::initPhonon() {
    // Phonon initialization
    if (mediaObject) delete mediaObject;
    if (audioOutput) delete audioOutput;
    mediaObject = new Phonon::MediaObject(this);
    mediaObject->setTickInterval(100);
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(totalTimeChanged(qint64)), this, SLOT(totalTimeChanged(qint64)));
    seekSlider->setMediaObject(mediaObject);
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    connect(audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
    connect(audioOutput, SIGNAL(mutedChanged(bool)), this, SLOT(volumeMutedChanged(bool)));
    volumeSlider->setAudioOutput(audioOutput);
    Phonon::createPath(mediaObject, audioOutput);
}

void MainWindow::tick(qint64 time) {
    if (time <= 0) {
        currentTime->clear();
        return;
    }

    currentTime->setText(formatTime(time));

    // remaining time
    const qint64 remainingTime = mediaObject->remainingTime();
    currentTime->setStatusTip(tr("Remaining time: %1").arg(formatTime(remainingTime)));

    /*
    slider->blockSignals(true);
    slider->setValue(time/1000);
    slider->blockSignals(false);
    */
}

void MainWindow::totalTimeChanged(qint64 time) {
    if (time <= 0) {
        totalTime->clear();
        return;
    }
    totalTime->setText(formatTime(time));

    /*
    slider->blockSignals(true);
    slider->setMaximum(time/1000);
    slider->blockSignals(false);
    */

}

QString MainWindow::formatTime(qint64 time) {
    QTime displayTime;
    displayTime = displayTime.addMSecs(time);
    QString timeString;
    // 60 * 60 * 1000 = 3600000
    if (time > 3600000)
        timeString = displayTime.toString("h:mm:ss");
    else
        timeString = displayTime.toString("m:ss");
    return timeString;
}

void MainWindow::volumeUp() {
    qreal newVolume = volumeSlider->audioOutput()->volume() + .1;
    if (newVolume > volumeSlider->maximumVolume())
        newVolume = volumeSlider->maximumVolume();
    volumeSlider->audioOutput()->setVolume(newVolume);
}

void MainWindow::volumeDown() {
    qreal newVolume = volumeSlider->audioOutput()->volume() - .1;
    if (newVolume < 0)
        newVolume = 0;
    volumeSlider->audioOutput()->setVolume(newVolume);
}

void MainWindow::volumeMute() {
    volumeSlider->audioOutput()->setMuted(!volumeSlider->audioOutput()->isMuted());
}

void MainWindow::volumeChanged(qreal newVolume) {
    // automatically unmute when volume changes
    if (volumeSlider->audioOutput()->isMuted())
        volumeSlider->audioOutput()->setMuted(false);
    statusBar()->showMessage(tr("Volume at %1%").arg(newVolume*100));
}

void MainWindow::volumeMutedChanged(bool muted) {
    if (muted) {
        volumeMuteAct->setIcon(QtIconLoader::icon("audio-volume-muted"));
        statusBar()->showMessage(tr("Volume is muted"));
    } else {
        volumeMuteAct->setIcon(QtIconLoader::icon("audio-volume-high"));
        statusBar()->showMessage(tr("Volume is unmuted"));
    }
}

void MainWindow::setDefinitionMode(QString definitionName) {
    QAction *definitionAct = The::globalActions()->value("definition");
    definitionAct->setText(definitionName);
    definitionAct->setStatusTip(tr("Maximum video definition set to %1").arg(definitionAct->text())
                                + " (" +  definitionAct->shortcut().toString(QKeySequence::NativeText) + ")");
    statusBar()->showMessage(definitionAct->statusTip());
    QSettings settings;
    settings.setValue("definition", definitionName);
}

void MainWindow::toggleDefinitionMode() {
    QSettings settings;
    QString currentDefinition = settings.value("definition").toString();
    QStringList definitionNames = VideoDefinition::getDefinitionNames();
    int currentIndex = definitionNames.indexOf(currentDefinition);
    int nextIndex = 0;
    if (currentIndex != definitionNames.size() - 1) {
        nextIndex = currentIndex + 1;
    }
    QString nextDefinition = definitionNames.at(nextIndex);
    setDefinitionMode(nextDefinition);
}

void MainWindow::showFullscreenToolbar(bool show) {
    if (!m_fullscreen) return;
    mainToolBar->setVisible(show);
}

void MainWindow::showFullscreenPlaylist(bool show) {
    if (!m_fullscreen) return;
    mediaView->setPlaylistVisible(show);
}

void MainWindow::clearRecentKeywords() {
    QSettings settings;
    settings.remove("recentKeywords");
    searchView->updateRecentKeywords();
    statusBar()->showMessage(tr("Your privacy is now safe"));
}

/*
 void MainWindow::setAutoplay(bool enabled) {
     QSettings settings;
     settings.setValue("autoplay", QVariant::fromValue(enabled));
 }
 */

void MainWindow::updateDownloadMessage(QString message) {
    The::globalActions()->value("downloads")->setText(message);
}

void MainWindow::downloadsFinished() {
    The::globalActions()->value("downloads")->setText(tr("&Downloads"));
    statusBar()->showMessage(tr("Downloads complete"));
}

void MainWindow::toggleDownloads(bool show) {

    if (show) {
        stopAct->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_MediaStop));
        The::globalActions()->value("downloads")->setShortcuts(
                QList<QKeySequence>() << QKeySequence(Qt::CTRL + Qt::Key_J)
                << QKeySequence(Qt::Key_Escape));
    } else {
        The::globalActions()->value("downloads")->setShortcuts(
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

void MainWindow::startToolbarSearch(QString query) {

    query = query.trimmed();

    // check for empty query
    if (query.length() == 0) {
        return;
    }

    SearchParams *searchParams = new SearchParams();
    searchParams->setKeywords(query);

    // go!
    showMedia(searchParams);
}
