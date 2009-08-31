#include "MediaView.h"
#include "playlist/PrettyItemDelegate.h"
#include "networkaccess.h"
#include "videowidget.h"
#include "minisplitter.h"

namespace The {
    QMap<QString, QAction*>* globalActions();
    QMap<QString, QMenu*>* globalMenus();
    QNetworkAccessManager* networkAccessManager();
}

MediaView::MediaView(QWidget *parent) : QWidget(parent) {

    QBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);

    splitter = new MiniSplitter(this);
    splitter->setChildrenCollapsible(false);

    sortBar = new THBlackBar(this);
    mostRelevantAction = new QAction(tr("Most relevant"), this);
    QKeySequence keySequence(Qt::CTRL + Qt::Key_1);
    mostRelevantAction->setShortcut(keySequence);
    mostRelevantAction->setStatusTip(keySequence.toString(QKeySequence::NativeText));
    addAction(mostRelevantAction);
    connect(mostRelevantAction, SIGNAL(triggered()), this, SLOT(searchMostRelevant()), Qt::QueuedConnection);
    sortBar->addAction(mostRelevantAction);
    mostRecentAction = new QAction(tr("Most recent"), this);
    keySequence = QKeySequence(Qt::CTRL + Qt::Key_2);
    mostRecentAction->setShortcut(keySequence);
    mostRecentAction->setStatusTip(keySequence.toString(QKeySequence::NativeText));
    addAction(mostRecentAction);
    connect(mostRecentAction, SIGNAL(triggered()), this, SLOT(searchMostRecent()), Qt::QueuedConnection);
    sortBar->addAction(mostRecentAction);
    mostViewedAction = new QAction(tr("Most viewed"), this);
    keySequence = QKeySequence(Qt::CTRL + Qt::Key_3);
    mostViewedAction->setShortcut(keySequence);
    mostViewedAction->setStatusTip(keySequence.toString(QKeySequence::NativeText));
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

    videoWidget = new Phonon::VideoWidget(this);
    videoAreaWidget->setVideoWidget(videoWidget);
    videoAreaWidget->setListModel(listModel);

    loadingWidget = new LoadingWidget(this);
    videoAreaWidget->setLoadingWidget(loadingWidget);

    splitter->addWidget(videoAreaWidget);

    layout->addWidget(splitter);
    setLayout(layout);

    errorTimer = new QTimer(this);
    errorTimer->setSingleShot(true);
    errorTimer->setInterval(3000);
    connect(errorTimer, SIGNAL(timeout()), SLOT(skipVideo()));
}

MediaView::~MediaView() {

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
    // connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
    connect(mediaObject, SIGNAL(finished()), this, SLOT(skip()));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(currentSourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(bufferStatus(int)), loadingWidget, SLOT(bufferStatus(int)));
}

void MediaView::search(SearchParams *searchParams) {
    this->searchParams = searchParams;

    // start serching for videos
    listModel->search(searchParams);

    // this implies that the enum and the bar action order is the same
    sortBar->setCheckedAction(searchParams->sortBy()-1);

    listView->setFocus();

    loadingWidget->clear();
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

    switch (newState) {

         case Phonon::ErrorState:
        qDebug() << "Phonon error:" << mediaObject->errorString() << mediaObject->errorType();
        handleError(mediaObject->errorString());
        break;

         case Phonon::PlayingState:
        qDebug("playing");
        videoAreaWidget->showVideo();
        break;

         case Phonon::StoppedState:
        qDebug("stopped");
        // play() has already been called when setting the source
        // but Phonon on Linux needs a little more help to start playback
        mediaObject->play();

        // Workaround for Mac playback start problem
        if (!timerPlayFlag) {
            QTimer::singleShot(1000, this, SLOT(timerPlay()));
        }

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
    mediaObject->stop();
    mediaObject->clear();
}

void MediaView::activeRowChanged(int row) {
    Video *video = listModel->videoAt(row);
    if (!video) return;

    // immediately show the loading widget
    videoAreaWidget->showLoading(video);

    // mediaObject->pause();

    connect(video, SIGNAL(gotStreamUrl(QUrl)), SLOT(gotStreamUrl(QUrl)));
    // TODO handle signal in a proper slot and impl item error status
    connect(video, SIGNAL(errorStreamUrl(QString)), SLOT(handleError(QString)));
    video->loadStreamUrl();

    // reset the timer flag
    timerPlayFlag = false;

    // video title in the statusbar
    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(qApp->topLevelWidgets().first());
    if (mainWindow) mainWindow->statusBar()->showMessage(video->title());

    // see you in gotStreamUrl...

}

void MediaView::gotStreamUrl(QUrl streamUrl) {

    // go!
    mediaObject->setCurrentSource(streamUrl);
    mediaObject->play();

    // ensure we always have 10 videos ahead
    listModel->searchNeeded();

    // ensure active item is visible
    int row = listModel->activeRow();
    if (row != -1) {
        QModelIndex index = listModel->index(row, 0, QModelIndex());
        listView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }

}

void MediaView::itemActivated(const QModelIndex &index) {
    if (listModel->rowExists(index.row()))
        listModel->setActiveRow(index.row());
    // the user doucleclicked on the "Search More" item
    else listModel->searchMore();
}

void MediaView::aboutToFinish() {
    /*
    int nextRow = listModel->nextRow();
    if (nextRow == -1) return;
    Video* video = listModel->videoAt(nextRow);
    QUrl streamUrl = video->streamUrl();
    qDebug() << "Enqueing" << streamUrl;
    mediaObject->enqueue(streamUrl);
    */
}

void MediaView::currentSourceChanged(const Phonon::MediaSource source) {
    qDebug() << "Source changed:" << source.url();
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

void MediaView::openWebPage() {
    Video* video = listModel->activeVideo();
    if (!video) return;
    mediaObject->pause();
    QDesktopServices::openUrl(video->webpage());
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
    listModel->move(indexes, true);
}

void MediaView::moveDownSelected() {
    if (!listView->selectionModel()->hasSelection()) return;
    QModelIndexList indexes = listView->selectionModel()->selectedIndexes();
    listModel->move(indexes, false);
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
    // qDebug() << mediaObject->currentTime();
    // Workaround Phonon bug on Mac OSX
    if (mediaObject->currentTime() <= 0 && mediaObject->state() == Phonon::PlayingState) {
        mediaObject->pause();
        mediaObject->play();
    }
}
