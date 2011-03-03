#include "MediaView.h"
#include "playlist/PrettyItemDelegate.h"
#include "networkaccess.h"
#include "videowidget.h"
#include "minisplitter.h"
#include "constants.h"
#include "downloadmanager.h"
#include "downloaditem.h"
#include "MainWindow.h"

namespace The {
    NetworkAccess* http();
}

namespace The {
    QMap<QString, QAction*>* globalActions();
    QMap<QString, QMenu*>* globalMenus();
    QNetworkAccessManager* networkAccessManager();
}

MediaView::MediaView(QWidget *parent) : QWidget(parent) {

    reallyStopped = false;
    downloadItem = 0;

    QBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);

    splitter = new MiniSplitter(this);
    splitter->setChildrenCollapsible(false);

    sortBar = new THBlackBar(this);
    mostRelevantAction = new QAction(tr("Most relevant"), this);
    QKeySequence keySequence(Qt::CTRL + Qt::Key_1);
    mostRelevantAction->setShortcut(keySequence);
    mostRelevantAction->setStatusTip(mostRelevantAction->text() + " (" + keySequence.toString(QKeySequence::NativeText) + ")");
    addAction(mostRelevantAction);
    connect(mostRelevantAction, SIGNAL(triggered()), this, SLOT(searchMostRelevant()), Qt::QueuedConnection);
    sortBar->addAction(mostRelevantAction);
    mostRecentAction = new QAction(tr("Most recent"), this);
    keySequence = QKeySequence(Qt::CTRL + Qt::Key_2);
    mostRecentAction->setShortcut(keySequence);
    mostRecentAction->setStatusTip(mostRecentAction->text() + " (" + keySequence.toString(QKeySequence::NativeText) + ")");
    addAction(mostRecentAction);
    connect(mostRecentAction, SIGNAL(triggered()), this, SLOT(searchMostRecent()), Qt::QueuedConnection);
    sortBar->addAction(mostRecentAction);
    mostViewedAction = new QAction(tr("Most viewed"), this);
    keySequence = QKeySequence(Qt::CTRL + Qt::Key_3);
    mostViewedAction->setShortcut(keySequence);
    mostViewedAction->setStatusTip(mostViewedAction->text() + " (" + keySequence.toString(QKeySequence::NativeText) + ")");
    addAction(mostViewedAction);
    connect(mostViewedAction, SIGNAL(triggered()), this, SLOT(searchMostViewed()), Qt::QueuedConnection);
    sortBar->addAction(mostViewedAction);

    listView = new QListView(this);
    listView->setItemDelegate(new PrettyItemDelegate(this));
    listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // dragndrop
    listView->setDragEnabled(true);
    listView->setAcceptDrops(true);
    listView->setDropIndicatorShown(true);
    listView->setDragDropMode(QAbstractItemView::DragDrop);

    // cosmetics
    listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    listView->setFrameShape( QFrame::NoFrame );
    listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    listView->setMinimumSize(320,240);
    listView->setUniformItemSizes(true);

    // respond to the user doubleclicking a playlist item
    connect(listView, SIGNAL(activated(const QModelIndex &)), this, SLOT(itemActivated(const QModelIndex &)));

    listModel = new ListModel(this);
    connect(listModel, SIGNAL(activeRowChanged(int)), this, SLOT(activeRowChanged(int)));
    // needed to restore the selection after dragndrop
    connect(listModel, SIGNAL(needSelectionFor(QList<Video*>)), this, SLOT(selectVideos(QList<Video*>)));
    listView->setModel(listModel);

    connect(listView->selectionModel(),
            SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection & )),
            this, SLOT(selectionChanged ( const QItemSelection & , const QItemSelection & )));

    playlistWidget = new PlaylistWidget(this, sortBar, listView);

    splitter->addWidget(playlistWidget);

    videoAreaWidget = new VideoAreaWidget(this);
    videoAreaWidget->setMinimumSize(320,240);

#ifdef APP_MAC
    // mouse autohide does not work on the Mac (no mouseMoveEvent)
    videoWidget = new Phonon::VideoWidget(this);
#else
    videoWidget = new VideoWidget(this);
#endif

    videoAreaWidget->setVideoWidget(videoWidget);
    videoAreaWidget->setListModel(listModel);

    loadingWidget = new LoadingWidget(this);
    videoAreaWidget->setLoadingWidget(loadingWidget);

    splitter->addWidget(videoAreaWidget);

    layout->addWidget(splitter);
    setLayout(layout);

    // restore splitter state
    QSettings settings;
    splitter->restoreState(settings.value("splitter").toByteArray());

    errorTimer = new QTimer(this);
    errorTimer->setSingleShot(true);
    errorTimer->setInterval(3000);
    connect(errorTimer, SIGNAL(timeout()), SLOT(skipVideo()));

    workaroundTimer = new QTimer(this);
    workaroundTimer->setSingleShot(true);
    workaroundTimer->setInterval(3000);
    connect(workaroundTimer, SIGNAL(timeout()), SLOT(timerPlay()));

#ifdef APP_DEMO
    demoTimer = new QTimer(this);
    demoTimer->setSingleShot(true);
    demoTimer->setInterval(60000);
    connect(demoTimer, SIGNAL(timeout()), SLOT(demoMessage()));
#endif

}

void MediaView::initialize() {
    connect(videoAreaWidget, SIGNAL(doubleClicked()), The::globalActions()->value("fullscreen"), SLOT(trigger()));
    videoAreaWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(videoAreaWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showVideoContextMenu(QPoint)));
}

void MediaView::setMediaObject(Phonon::MediaObject *mediaObject) {
    this->mediaObject = mediaObject;
    Phonon::createPath(this->mediaObject, videoWidget);
    connect(mediaObject, SIGNAL(finished()), this, SLOT(playbackFinished()));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(currentSourceChanged(Phonon::MediaSource)));
    // connect(mediaObject, SIGNAL(bufferStatus(int)), loadingWidget, SLOT(bufferStatus(int)));
}

void MediaView::search(SearchParams *searchParams) {
    reallyStopped = false;

#ifdef APP_DEMO
    demoTimer->stop();
#endif

    videoAreaWidget->clear();
    workaroundTimer->stop();
    errorTimer->stop();

    mediaObject->pause();
    if (downloadItem) {
        delete downloadItem;
        downloadItem = 0;
    }

    this->searchParams = searchParams;

    // start serching for videos
    listModel->search(searchParams);

    // this implies that the enum and the bar action order is the same
    sortBar->setCheckedAction(searchParams->sortBy()-1);

    listView->setFocus();


    QString keyword = searchParams->keywords();
    QString display = keyword;
    if (keyword.startsWith("http://")) {
        int separator = keyword.indexOf("|");
        if (separator > 0 && separator + 1 < keyword.length()) {
            display = keyword.mid(separator+1);
        }

        // also hide sidebar
        playlistWidget->hide();
    } else playlistWidget->show();
    // tr("You're watching \"%1\"").arg(searchParams->keywords())

}

void MediaView::disappear() {
    timerPlayFlag = true;
}

void MediaView::handleError(QString message) {
    videoAreaWidget->showError(message);
    skippedVideo = listModel->activeVideo();
    // recover from errors by skipping to the next video
    errorTimer->start(2000);
}

void MediaView::stateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{

    // qDebug() << "Phonon state: " << newState << oldState;
    // slider->setEnabled(newState == Phonon::PlayingState);

    switch (newState) {

    case Phonon::ErrorState:
        qDebug() << "Phonon error:" << mediaObject->errorString() << mediaObject->errorType();
        if (mediaObject->errorType() == Phonon::FatalError)
            handleError(mediaObject->errorString());
        break;

    case Phonon::PlayingState:
        // qDebug("playing");
        videoAreaWidget->showVideo();
        break;

    case Phonon::StoppedState:
        // qDebug("stopped");
        // play() has already been called when setting the source
        // but Phonon on Linux needs a little more help to start playback
        // if (!reallyStopped) mediaObject->play();

#ifdef APP_MAC
        // Workaround for Mac playback start problem
        if (!timerPlayFlag) {
            // workaroundTimer->start();
        }
#endif

        break;

         case Phonon::PausedState:
        qDebug("paused");
        break;

         case Phonon::BufferingState:
        qDebug("buffering");
        break;

         case Phonon::LoadingState:
        qDebug("loading");
        break;

         default:
        ;
    }
}

void MediaView::pause() {
    // qDebug() << "pause() called" << mediaObject->state();
    switch( mediaObject->state() ) {
    case Phonon::PlayingState:
        mediaObject->pause();
        break;
    default:
        mediaObject->play();
        break;
    }
}

void MediaView::stop() {
    listModel->abortSearch();
    reallyStopped = true;
    mediaObject->stop();
    videoAreaWidget->clear();
    workaroundTimer->stop();
    errorTimer->stop();
    listView->selectionModel()->clearSelection();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
        downloadItem = 0;
    }
}

void MediaView::activeRowChanged(int row) {
    if (reallyStopped) return;

    Video *video = listModel->videoAt(row);
    if (!video) return;

    // now that we have a new video to play
    // stop all the timers
    workaroundTimer->stop();
    errorTimer->stop();

    mediaObject->pause();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
        downloadItem = 0;
    }
    // slider->setMinimum(0);

    // immediately show the loading widget
    videoAreaWidget->showLoading(video);

    connect(video, SIGNAL(gotStreamUrl(QUrl)), SLOT(gotStreamUrl(QUrl)));
    // TODO handle signal in a proper slot and impl item error status
    connect(video, SIGNAL(errorStreamUrl(QString)), SLOT(handleError(QString)));

    video->loadStreamUrl();

    // reset the timer flag
    timerPlayFlag = false;

    // video title in the statusbar
    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(window());
    if (mainWindow) mainWindow->statusBar()->showMessage(video->title());

    The::globalActions()->value("download")->setEnabled(DownloadManager::instance()->itemForVideo(video) == 0);

    // see you in gotStreamUrl...

}

void MediaView::gotStreamUrl(QUrl streamUrl) {
    if (reallyStopped) return;

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender";
        return;
    }
    video->disconnect(this);


    QString tempDir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#ifdef Q_WS_X11
    QString tempFile = tempDir + "/minitube-" + getenv("USERNAME") + ".mp4";
#else
    QString tempFile = tempDir + "/minitube.mp4";
#endif
    if (QFile::exists(tempFile) && !QFile::remove(tempFile)) {
        qDebug() << "Cannot remove temp file";
    }

    Video *videoCopy = video->clone();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
    }
    downloadItem = new DownloadItem(videoCopy, streamUrl, tempFile, this);
    connect(downloadItem, SIGNAL(statusChanged()), SLOT(downloadStatusChanged()));
    // connect(downloadItem, SIGNAL(progress(int)), SLOT(downloadProgress(int)));
    connect(downloadItem, SIGNAL(bufferProgress(int)), loadingWidget, SLOT(bufferStatus(int)));
    // connect(downloadItem, SIGNAL(finished()), SLOT(itemFinished()));
    connect(video, SIGNAL(errorStreamUrl(QString)), SLOT(handleError(QString)));
    connect(downloadItem, SIGNAL(error(QString)), SLOT(handleError(QString)));
    downloadItem->start();

}

/*
void MediaView::downloadProgress(int percent) {
    MainWindow* mainWindow = dynamic_cast<MainWindow*>(window());

    mainWindow->getSeekSlider()->setStyleSheet(" QSlider::groove:horizontal {"
        "border: 1px solid #999999;"
        // "border-left: 50px solid rgba(255, 0, 0, 128);"
        "height: 8px;"
        "background: qlineargradient(x1:0, y1:0, x2:.5, y2:0, stop:0 rgba(255, 0, 0, 92), stop:"
        + QString::number(percent/100.0) +

        " rgba(255, 0, 0, 92), stop:" + QString::number((percent+1)/100.0) + " transparent, stop:1 transparent);"
        "margin: 2px 0;"
    "}"
    "QSlider::handle:horizontal {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
        "border: 1px solid #5c5c5c;"
        "width: 16px;"
        "height: 16px;"
        "margin: -2px 0;"
        "border-radius: 8px;"
    "}"

    );
}

*/

void MediaView::downloadStatusChanged() {
    switch(downloadItem->status()) {
    case Downloading:
        startPlaying();
        break;
    case Starting:
        // qDebug() << "Starting";
        break;
    case Finished:
        // qDebug() << "Finished" << mediaObject->state();
        // if (mediaObject->state() == Phonon::StoppedState) startPlaying();
        break;
    case Failed:
        // qDebug() << "Failed";
    case Idle:
        // qDebug() << "Idle";
        break;
    }
}

void MediaView::startPlaying() {
    if (reallyStopped) return;

    // go!
    qDebug() << "Playing" << downloadItem->currentFilename();
    mediaObject->setCurrentSource(downloadItem->currentFilename());
    mediaObject->play();

    // ensure we always have 10 videos ahead
    listModel->searchNeeded();

    // ensure active item is visible
    int row = listModel->activeRow();
    if (row != -1) {
        QModelIndex index = listModel->index(row, 0, QModelIndex());
        listView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }

#ifdef APP_DEMO
    demoTimer->start();
#endif

}

void MediaView::itemActivated(const QModelIndex &index) {
    if (listModel->rowExists(index.row()))
        listModel->setActiveRow(index.row());
    // the user doubleclicked on the "Search More" item
    else listModel->searchMore();
}

void MediaView::currentSourceChanged(const Phonon::MediaSource /* source */ ) {

}

void MediaView::skipVideo() {
    // skippedVideo is useful for DELAYED skip operations
    // in order to be sure that we're skipping the video we wanted
    // and not another one
    if (skippedVideo) {
        if (listModel->activeVideo() != skippedVideo) {
            qDebug() << "Skip of video canceled";
            return;
        }
        int nextRow = listModel->rowForVideo(skippedVideo);
        nextRow++;
        if (nextRow == -1) return;
        listModel->setActiveRow(nextRow);
    }
}

void MediaView::skip() {
    int nextRow = listModel->nextRow();
    if (nextRow == -1) return;
    listModel->setActiveRow(nextRow);
}

void MediaView::playbackFinished() {
    if (mediaObject->currentTime() < mediaObject->totalTime()) {
        // mediaObject->seek(mediaObject->currentTime());
        QTimer::singleShot(3000, this, SLOT(playbackResume()));
    } else skip();
}

void MediaView::playbackResume() {
    mediaObject->seek(mediaObject->currentTime());
    mediaObject->play();
}

void MediaView::openWebPage() {
    Video* video = listModel->activeVideo();
    if (!video) return;
    mediaObject->pause();
    QDesktopServices::openUrl(video->webpage());
}

void MediaView::copyWebPage() {
    Video* video = listModel->activeVideo();
    if (!video) return;
    QString address = video->webpage().toString();
    address.remove("&feature=youtube_gdata");
    QApplication::clipboard()->setText(address);
    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(window());
    QString message = tr("You can now paste the YouTube link into another application");
    if (mainWindow) mainWindow->statusBar()->showMessage(message);
}

void MediaView::copyVideoLink() {
    Video* video = listModel->activeVideo();
    if (!video) return;
    QApplication::clipboard()->setText(video->getStreamUrl().toEncoded());
    QString message = tr("You can now paste the video stream URL into another application")
                      + ". " + tr("The link will be valid only for a limited time.");
    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(window());
    if (mainWindow) mainWindow->statusBar()->showMessage(message);
}

void MediaView::removeSelected() {
    if (!listView->selectionModel()->hasSelection()) return;
    QModelIndexList indexes = listView->selectionModel()->selectedIndexes();
    listModel->removeIndexes(indexes);
}

void MediaView::selectVideos(QList<Video*> videos) {
    foreach (Video *video, videos) {
        QModelIndex index = listModel->indexForVideo(video);
        listView->selectionModel()->select(index, QItemSelectionModel::Select);
        listView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }
}

void MediaView::selectionChanged(const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/) {
    const bool gotSelection = listView->selectionModel()->hasSelection();
    The::globalActions()->value("remove")->setEnabled(gotSelection);
    The::globalActions()->value("moveUp")->setEnabled(gotSelection);
    The::globalActions()->value("moveDown")->setEnabled(gotSelection);
}

void MediaView::moveUpSelected() {
    if (!listView->selectionModel()->hasSelection()) return;

    QModelIndexList indexes = listView->selectionModel()->selectedIndexes();
    qStableSort(indexes.begin(), indexes.end());
    listModel->move(indexes, true);

    // set current index after row moves to something more intuitive
    int row = indexes.first().row();
    listView->selectionModel()->setCurrentIndex(listModel->index(row>1?row:1), QItemSelectionModel::NoUpdate);
}

void MediaView::moveDownSelected() {
    if (!listView->selectionModel()->hasSelection()) return;

    QModelIndexList indexes = listView->selectionModel()->selectedIndexes();
    qStableSort(indexes.begin(), indexes.end(), qGreater<QModelIndex>());
    listModel->move(indexes, false);

    // set current index after row moves to something more intuitive (respect 1 static item on bottom)
    int row = indexes.first().row()+1, max = listModel->rowCount() - 2;
    listView->selectionModel()->setCurrentIndex(listModel->index(row>max?max:row), QItemSelectionModel::NoUpdate);
}

void MediaView::showVideoContextMenu(QPoint point) {
    The::globalMenus()->value("video")->popup(videoWidget->mapToGlobal(point));
}

void MediaView::searchMostRelevant() {
    searchParams->setSortBy(SearchParams::SortByRelevance);
    search(searchParams);
}

void MediaView::searchMostRecent() {
    searchParams->setSortBy(SearchParams::SortByNewest);
    search(searchParams);
}

void MediaView::searchMostViewed() {
    searchParams->setSortBy(SearchParams::SortByViewCount);
    search(searchParams);
}

void MediaView::setPlaylistVisible(bool visible) {
    playlistWidget->setVisible(visible);
}

void MediaView::timerPlay() {
    // Workaround Phonon bug on Mac OSX
    // qDebug() << mediaObject->currentTime();
    if (mediaObject->currentTime() <= 0 && mediaObject->state() == Phonon::PlayingState) {
        // qDebug() << "Mac playback workaround";
        mediaObject->pause();
        // QTimer::singleShot(1000, mediaObject, SLOT(play()));
        mediaObject->play();
    }
}

void MediaView::saveSplitterState() {
    QSettings settings;
    settings.setValue("splitter", splitter->saveState());
}

#ifdef APP_DEMO
void MediaView::demoMessage() {
    if (mediaObject->state() != Phonon::PlayingState) return;
    mediaObject->pause();

    QMessageBox msgBox;
    msgBox.setIconPixmap(QPixmap(":/images/app.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setText(tr("This is just the demo version of %1.").arg(Constants::APP_NAME));
    msgBox.setInformativeText(tr("It allows you to test the application and see if it works for you."));
    msgBox.setModal(true);

    QPushButton *quitButton = msgBox.addButton(tr("Continue"), QMessageBox::RejectRole);
    QPushButton *buyButton = msgBox.addButton(tr("Get the full version"), QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == buyButton) {
        QDesktopServices::openUrl(QString(Constants::WEBSITE) + "#download");
    } else {
        mediaObject->play();
        demoTimer->start(300000);
    }
}
#endif

void MediaView::downloadVideo() {
    Video* video = listModel->activeVideo();
    if (!video) return;

    DownloadManager::instance()->addItem(video);

    // TODO animate

    The::globalActions()->value("downloads")->setVisible(true);

    // The::globalActions()->value("download")->setEnabled(DownloadManager::instance()->itemForVideo(video) == 0);

    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(window());
    QString message = tr("Downloading %1").arg(video->title());
    if (mainWindow) mainWindow->statusBar()->showMessage(message);
}

void MediaView::fullscreen() {
    videoAreaWidget->setParent(0);
    videoAreaWidget->showFullScreen();
}

/*
void MediaView::setSlider(QSlider *slider) {
    this->slider = slider;
    // slider->setEnabled(false);
    slider->setTracking(false);
    // connect(slider, SIGNAL(valueChanged(int)), SLOT(sliderMoved(int)));
}

void MediaView::sliderMoved(int value) {
    qDebug() << __func__;
    int sliderPercent = (value * 100) / (slider->maximum() - slider->minimum());
    qDebug() << slider->minimum() << value << slider->maximum();
    if (sliderPercent <= downloadItem->currentPercent()) {
        qDebug() << sliderPercent << downloadItem->currentPercent();
        mediaObject->seek(value);
    } else {
        seekTo(value);
    }
}

void MediaView::seekTo(int value) {
    qDebug() << __func__;
    mediaObject->pause();
    workaroundTimer->stop();
    errorTimer->stop();
    // mediaObject->clear();

    QString tempDir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
    QString tempFile = tempDir + "/minitube" + QString::number(value) + ".mp4";
    if (!QFile::remove(tempFile)) {
        qDebug() << "Cannot remove temp file";
    }
    Video *videoCopy = downloadItem->getVideo()->clone();
    QUrl streamUrl = videoCopy->getStreamUrl();
    streamUrl.addQueryItem("begin", QString::number(value));
    if (downloadItem) delete downloadItem;
    downloadItem = new DownloadItem(videoCopy, streamUrl, tempFile, this);
    connect(downloadItem, SIGNAL(statusChanged()), SLOT(downloadStatusChanged()));
    // connect(downloadItem, SIGNAL(finished()), SLOT(itemFinished()));
    downloadItem->start();

    // slider->setMinimum(value);

}

*/
