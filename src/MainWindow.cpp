#include "MainWindow.h"
#include "spacer.h"
#include "Constants.h"
#include "iconloader/qticonloader.h"
#include "global.h"

MainWindow::MainWindow() {

    m_fullscreen = false;
    mediaObject = 0;
    audioOutput = 0;

    // views mechanism
    history = new QStack<QWidget*>();
    views = new QStackedWidget(this);

    // views
    searchView = new SearchView(this);
    connect(searchView, SIGNAL(search(QString)), this, SLOT(showMedia(QString)));
    views->addWidget(searchView);
    mediaView = new MediaView(this);
    views->addWidget(mediaView);

    // lazy initialized views
    aboutView = 0;
    settingsView = 0;

    toolbarSearch = new SearchLineEdit(this);
    toolbarSearch->setFont(qApp->font());
    // toolbarSearch->setMinimumWidth(200);
    connect(toolbarSearch, SIGNAL(search(const QString&)), searchView, SLOT(watch(const QString&)));

    // build ui
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    // remove that useless menu/toolbar context menu
    this->setContextMenuPolicy(Qt::NoContextMenu);

    // mediaView init stuff thats needs actions
    mediaView->initialize();

    // restore window position
    readSettings();

    // show the initial view
    showWidget(searchView);

    setCentralWidget(views);

    // top dock widget
    /*
    QLabel* message = new QLabel(this);
    message->setText(QString("A new version of %1 is available.").arg(Constants::APP_NAME));
    message->setMargin(10);
    message->setAlignment(Qt::AlignCenter);
    QPalette palette;
    message->setBackgroundRole(QPalette::ToolTipBase);
    message->setForegroundRole(QPalette::ToolTipText);
    message->setAutoFillBackground(true);
    QDockWidget *dockWidget = new QDockWidget("", this, 0);
    dockWidget->setTitleBarWidget(0);
    dockWidget->setWidget(message);
    dockWidget->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::TopDockWidgetArea, dockWidget);
    */

}

MainWindow::~MainWindow() {
    delete history;
}

void MainWindow::createActions() {

    QMap<QString, QAction*> *actions = The::globalActions();

    /*
    settingsAct = new QAction(tr("&Preferences..."), this);
    settingsAct->setStatusTip(tr(QString("Configure ").append(Constants::APP_NAME).toUtf8()));
    // Mac integration
    settingsAct->setMenuRole(QAction::PreferencesRole);
    actions->insert("settings", settingsAct);
    connect(settingsAct, SIGNAL(triggered()), this, SLOT(showSettings()));
    */
    
    backAct = new QAction(QIcon(":/images/go-previous.png"), tr("&Back"), this);
    backAct->setEnabled(false);
    backAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
    backAct->setStatusTip(tr("Go to the previous view"));
    actions->insert("back", backAct);
    connect(backAct, SIGNAL(triggered()), this, SLOT(goBack()));

    stopAct = new QAction(QtIconLoader::icon("media-stop", QIcon(":/images/stop.png")), tr("&Stop"), this);
    stopAct->setStatusTip(tr("Stop playback and go back to the search view"));
    stopAct->setShortcut(QKeySequence(Qt::Key_Escape));
    actions->insert("stop", stopAct);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(stop()));

    skipAct = new QAction(QtIconLoader::icon("media-skip-forward", QIcon(":/images/skip.png")), tr("S&kip"), this);
    skipAct->setStatusTip(tr("Skip to the next video"));
    skipAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right));
    skipAct->setEnabled(false);
    actions->insert("skip", skipAct);
    connect(skipAct, SIGNAL(triggered()), mediaView, SLOT(skip()));

    pauseAct = new QAction(QtIconLoader::icon("media-pause", QIcon(":/images/pause.png")), tr("&Pause"), this);
    pauseAct->setStatusTip(tr("Pause playback"));
    pauseAct->setShortcut(QKeySequence(Qt::Key_Space));
    pauseAct->setEnabled(false);
    actions->insert("pause", pauseAct);
    connect(pauseAct, SIGNAL(triggered()), mediaView, SLOT(pause()));

    fullscreenAct = new QAction(QtIconLoader::icon("view-fullscreen", QIcon(":/images/view-fullscreen.png")), tr("&Full Screen"), this);
    fullscreenAct->setStatusTip(tr("Go full screen"));
    fullscreenAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
    fullscreenAct->setShortcutContext(Qt::ApplicationShortcut);
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

    /*
    // icon should be document-save but it is ugly
    downloadAct = new QAction(QtIconLoader::icon("go-down", QIcon(":/images/go-down.png")), tr("&Download"), this);
    downloadAct->setStatusTip(tr("Download this video"));
    downloadAct->setShortcut(tr("Ctrl+S"));
    actions.insert("download", downloadAct);
    connect(downloadAct, SIGNAL(triggered()), this, SLOT(download()));
    */

    webPageAct = new QAction(QtIconLoader::icon("internet-web-browser", QIcon(":/images/internet-web-browser.png")), tr("&YouTube"), this);
    webPageAct->setStatusTip(tr("Open the YouTube video page"));
    webPageAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
    webPageAct->setEnabled(false);
    actions->insert("webpage", webPageAct);
    connect(webPageAct, SIGNAL(triggered()), mediaView, SLOT(openWebPage()));

    removeAct = new QAction(tr("&Remove"), this);
    removeAct->setStatusTip(tr("Remove the selected videos from the playlist"));
    QList<QKeySequence> shortcuts;
    shortcuts << QKeySequence("Del") << QKeySequence("Backspace");
    removeAct->setShortcuts(shortcuts);
    removeAct->setEnabled(false);
    actions->insert("remove", removeAct);
    connect(removeAct, SIGNAL(triggered()), mediaView, SLOT(removeSelected()));

    moveUpAct = new QAction(QtIconLoader::icon("go-up", QIcon(":/images/go-up.png")), tr("Move &Up"), this);
    moveUpAct->setStatusTip(tr("Move up the selected videos in the playlist"));
    moveUpAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up));
    moveUpAct->setEnabled(false);
    actions->insert("moveUp", moveUpAct);
    connect(moveUpAct, SIGNAL(triggered()), mediaView, SLOT(moveUpSelected()));

    moveDownAct = new QAction(QtIconLoader::icon("go-down", QIcon(":/images/go-down.png")), tr("Move &Down"), this);
    moveDownAct->setStatusTip(tr("Move down the selected videos in the playlist"));
    moveDownAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down));
    moveDownAct->setEnabled(false);
    actions->insert("moveDown", moveDownAct);
    connect(moveDownAct, SIGNAL(triggered()), mediaView, SLOT(moveDownSelected()));

    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setMenuRole(QAction::QuitRole);
    quitAct->setShortcut(tr("Ctrl+Q"));
    quitAct->setStatusTip(tr("Bye"));
    actions->insert("quit", quitAct);
    connect(quitAct, SIGNAL(triggered()), this, SLOT(quit()));

    siteAct = new QAction(tr("&Website"), this);
    siteAct->setShortcut(QKeySequence::HelpContents);
    siteAct->setStatusTip(tr("%1 on the Web").arg(Constants::APP_NAME));
    actions->insert("site", siteAct);
    connect(siteAct, SIGNAL(triggered()), this, SLOT(visitSite()));

    donateAct = new QAction(tr("&Donate via PayPal"), this);
    donateAct->setStatusTip(tr("Please support the continued development of %1").arg(Constants::APP_NAME));
    actions->insert("donate", donateAct);
    connect(donateAct, SIGNAL(triggered()), this, SLOT(donate()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setMenuRole(QAction::AboutRole);
    aboutAct->setStatusTip(tr("Info about %1").arg(Constants::APP_NAME));
    actions->insert("about", aboutAct);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    searchFocusAct = new QAction(tr("&Search"), this);
    searchFocusAct->setShortcut(QKeySequence::Find);
    actions->insert("search", searchFocusAct);
    connect(searchFocusAct, SIGNAL(triggered()), this, SLOT(searchFocus()));
    addAction(searchFocusAct);

    // common action properties
    foreach (QAction *action, actions->values()) {

        // add actions to the MainWindow so that they work
        // when the menu is hidden
        addAction(action);

        // never autorepeat.
        // unexperienced users tend to keep keys pressed for a "long" time
        action->setAutoRepeat(false);
        action->setToolTip(action->statusTip());

        // make the actions work when video is fullscreen
        action->setShortcutContext(Qt::ApplicationShortcut);

#ifdef Q_WS_MAC
        // OSX does not use icons in menus
        action->setIconVisibleInMenu(false);
#endif

    }

}

void MainWindow::createMenus() {

    QMap<QString, QMenu*> *menus = The::globalMenus();

    fileMenu = menuBar()->addMenu(tr("&Application"));
    // menus->insert("file", fileMenu);
    /*
    fileMenu->addAction(settingsAct);
    fileMenu->addSeparator();
    */
    fileMenu->addAction(quitAct);

    playlistMenu = menuBar()->addMenu(tr("&Playlist"));
    menus->insert("playlist", playlistMenu);
    playlistMenu->addAction(removeAct);
    playlistMenu->addSeparator();
    playlistMenu->addAction(moveUpAct);
    playlistMenu->addAction(moveDownAct);

    viewMenu = menuBar()->addMenu(tr("&Video"));
    menus->insert("video", viewMenu);
    // viewMenu->addAction(backAct);
    viewMenu->addAction(stopAct);
    viewMenu->addAction(pauseAct);
    viewMenu->addAction(skipAct);
    viewMenu->addSeparator();
    viewMenu->addAction(webPageAct);
    viewMenu->addSeparator();
    viewMenu->addAction(compactViewAct);
    viewMenu->addAction(fullscreenAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(siteAct);
    helpMenu->addAction(donateAct);
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars() {

    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mainToolBar = new QToolBar(this);
    mainToolBar->setFloatable(false);
    mainToolBar->setMovable(false);

    QFont smallerFont;
    smallerFont.setPointSize(smallerFont.pointSize()*.85);
    mainToolBar->setFont(smallerFont);

    mainToolBar->setIconSize(QSize(32,32));
    // mainToolBar->addAction(backAct);
    mainToolBar->addAction(stopAct);
    mainToolBar->addAction(pauseAct);
    mainToolBar->addAction(skipAct);
    mainToolBar->addAction(fullscreenAct);

    seekSlider = new Phonon::SeekSlider(this);
    seekSlider->setIconVisible(false);
    Spacer *seekSliderSpacer = new Spacer(mainToolBar, seekSlider);
    seekSliderSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainToolBar->addWidget(seekSliderSpacer);

    volumeSlider = new Phonon::VolumeSlider(this);
    // this makes the volume slider smaller
    volumeSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainToolBar->addWidget(new Spacer(mainToolBar, volumeSlider));

    mainToolBar->addWidget(new Spacer(mainToolBar, toolbarSearch));

    addToolBar(mainToolBar);
}

void MainWindow::createStatusBar() {
    currentTime = new QLabel(this);
    statusBar()->addPermanentWidget(currentTime);

    totalTime = new QLabel(this);
    statusBar()->addPermanentWidget(totalTime);

    // remove ugly borders on OSX
    statusBar()->setStyleSheet("::item{border:0 solid}");

    statusBar()->show();
}

void MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
}

void MainWindow::writeSettings() {
    // do not save geometry when in full screen
    if (m_fullscreen)
        return;
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
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
    if (oldView != NULL) {
        oldView->disappear();
    }

    // call show method on the new view
    View* newView = dynamic_cast<View *> (widget);
    if (newView != NULL) {
        newView->appear();
        QMap<QString,QVariant> metadata = newView->metadata();
        QString windowTitle = metadata.value("title").toString();
        if (windowTitle.length())
            windowTitle += " - ";
        setWindowTitle(windowTitle + Constants::APP_NAME);
        statusBar()->showMessage((metadata.value("description").toString()));

    }

    // backAct->setEnabled(history->size() > 1);
    // settingsAct->setEnabled(widget != settingsView);
    stopAct->setEnabled(widget == mediaView);
    fullscreenAct->setEnabled(widget == mediaView);
    compactViewAct->setEnabled(widget == mediaView);
    webPageAct->setEnabled(widget == mediaView);
    aboutAct->setEnabled(widget != aboutView);

    // cool toolbar on the Mac
    // setUnifiedTitleAndToolBarOnMac(widget == mediaView);

    // toolbar only for the mediaView
    mainToolBar->setVisible(widget == mediaView && !compactViewAct->isChecked());

    history->push(widget);
    fadeInWidget(views->currentWidget(), widget);
    views->setCurrentWidget(widget);

    setUpdatesEnabled(true);
}

void MainWindow::fadeInWidget(QWidget *oldWidget, QWidget *newWidget) {
    if (faderWidget) faderWidget->close();
    if (!oldWidget || !newWidget || oldWidget == mediaView || newWidget == mediaView) return;
    QPixmap frozenView = QPixmap::grabWidget(oldWidget);
    faderWidget = new FaderWidget(newWidget);
    faderWidget->start(frozenView);
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
    quit();
    QWidget::closeEvent(event);
}

void MainWindow::showSettings() {
    if (!settingsView) {
        settingsView = new SettingsView(this);
        views->addWidget(settingsView);
    }
    showWidget(settingsView);
}

void MainWindow::showSearch() {
    showWidget(searchView);
    currentTime->clear();
    totalTime->clear();
}

void MainWindow::showMedia(QString query) {
    initPhonon();
    mediaView->setMediaObject(mediaObject);
    SearchParams *searchParams = new SearchParams();
    searchParams->setKeywords(query);
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
        pauseAct->setIcon(QtIconLoader::icon("media-pause", QIcon(":/images/pause.png")));
        pauseAct->setText(tr("&Pause"));
        pauseAct->setStatusTip(tr("Pause playback"));
        skipAct->setEnabled(true);
        break;

         case Phonon::StoppedState:
        pauseAct->setEnabled(false);
        skipAct->setEnabled(false);
        break;

         case Phonon::PausedState:
        skipAct->setEnabled(true);
        pauseAct->setEnabled(true);
        pauseAct->setIcon(QtIconLoader::icon("media-play", QIcon(":/images/play.png")));
        pauseAct->setText(tr("&Play"));
        pauseAct->setStatusTip(tr("Resume playback"));
        break;

         case Phonon::BufferingState:
         case Phonon::LoadingState:
        skipAct->setEnabled(true);
        pauseAct->setEnabled(false);
        currentTime->clear();
        totalTime->clear();
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

    if (m_fullscreen) {
        fullscreenAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
        fullscreenAct->setText(tr("&Full Screen"));
        stopAct->setShortcut(QKeySequence(Qt::Key_Escape));
        if (m_maximized) showMaximized();
        else showNormal();
    } else {
        stopAct->setShortcut(QString(""));
        QList<QKeySequence> shortcuts;
        // for some reason it is important that ESC comes first
        shortcuts << QKeySequence(Qt::Key_Escape) << QKeySequence(Qt::ALT + Qt::Key_Return);
        fullscreenAct->setShortcuts(shortcuts);
        fullscreenAct->setText(tr("Exit &Full Screen"));
        m_maximized = isMaximized();

        // save geometry now, if the user quits when in full screen
        // geometry won't be saved
        writeSettings();

        showFullScreen();
    }

    // No compact view action when in full screen
    compactViewAct->setVisible(m_fullscreen);

    // Hide anything but the video
    mediaView->setPlaylistVisible(m_fullscreen);
    mainToolBar->setVisible(m_fullscreen);
    statusBar()->setVisible(m_fullscreen);
    menuBar()->setVisible(m_fullscreen);

    m_fullscreen = !m_fullscreen;

}

void MainWindow::compactView(bool enable) {

    // setUnifiedTitleAndToolBarOnMac(!enable);
    mediaView->setPlaylistVisible(!enable);
    mainToolBar->setVisible(!enable);
    statusBar()->setVisible(!enable);

    // ensure focus does not end up to the search box
    // as it would steal the Space shortcut
    toolbarSearch->setEnabled(!enable);

    if (enable) {
        stopAct->setShortcut(QString(""));
        QList<QKeySequence> shortcuts;
        // for some reason it is important that ESC comes first
        shortcuts << QKeySequence(Qt::CTRL + Qt::Key_Return) << QKeySequence(Qt::Key_Escape);
        compactViewAct->setShortcuts(shortcuts);
    } else {
        compactViewAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
        stopAct->setShortcut(QKeySequence(Qt::Key_Escape));
    }

}

void MainWindow::searchFocus() {
    QWidget *view = views->currentWidget();
    if (view == mediaView) {
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
    volumeSlider->setAudioOutput(audioOutput);
    Phonon::createPath(mediaObject, audioOutput);
}

void MainWindow::tick(qint64 time) {
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);
    currentTime->setText(displayTime.toString("mm:ss"));
    // qDebug() << "currentTime" << time << displayTime.toString("mm:ss");
}

void MainWindow::totalTimeChanged(qint64 time) {
    if (time <= 0) {
        totalTime->clear();
        return;
    }
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);
    totalTime->setText(displayTime.toString("/ mm:ss"));
    // qDebug() << "totalTime" << time << displayTime.toString("mm:ss");
}

