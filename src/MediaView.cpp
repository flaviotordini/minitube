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
    mostRelevantAction = new THAction(tr("Most relevant"), this);
    connect(mostRelevantAction, SIGNAL(triggered()), this, SLOT(searchMostRelevant()), Qt::QueuedConnection);
    sortBar->addAction(mostRelevantAction);
    mostRecentAction = new THAction(tr("Most recent"), this);
    connect(mostRecentAction, SIGNAL(triggered()), this, SLOT(searchMostRecent()), Qt::QueuedConnection);
    sortBar->addAction(mostRecentAction);
    mostViewedAction = new THAction(tr("Most viewed"), this);
    connect(mostViewedAction, SIGNAL(triggered()), this, SLOT(searchMostViewed()), Qt::QueuedConnection);
    sortBar->addAction(mostViewedAction);

    listView = new QListView(this);
    listView->setItemDelegate(new Playlist::PrettyItemDelegate(this));
    listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // dragndrop
    listView->setDragEnabled(true);
    listView->setAcceptDrops(true);
    listView->setDropIndicatorShown(true);
    listView->setDragDropMode(QAbstractItemView::InternalMove);

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
    // playlistWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    splitter->addWidget(playlistWidget);

    videoWidget = new VideoWidget(this);
    videoWidget->setMinimumSize(320,240);
    // videoWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    videoWidget->hide();
    splitter->addWidget(videoWidget);
    
    loadingWidget = new LoadingWidget(this);
    loadingWidget->setMinimumSize(320,240);
    // loadingWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    splitter->addWidget(loadingWidget);

    QList<int> sizes;
    sizes << 320 << 640 << 640;
    splitter->setSizes(sizes);

    layout->addWidget(splitter);
    setLayout(layout);
}

MediaView::~MediaView() {

}

void MediaView::initialize() {
    connect(videoWidget, SIGNAL(doubleClicked()), The::globalActions()->value("fullscreen"), SLOT(trigger()));
    videoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(videoWidget, SIGNAL(customContextMenuRequested(QPoint)),
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
    // connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(bufferStatus(int)), loadingWidget, SLOT(bufferStatus(int)));
}

void MediaView::search(SearchParams *searchParams) {
    this->searchParams = searchParams;

    // this implies that the enum and the bar action order is the same
    sortBar->setCheckedAction(searchParams->sortBy()-1);

    listModel->search(searchParams);
    listView->setFocus();
}

void MediaView::disappear() {

}

void MediaView::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
    switch (newState) {

         case Phonon::ErrorState:
        qDebug() << "Phonon error:" << mediaObject->errorString() << mediaObject->errorType();
        // recover from errors by skipping to the next video
        skip();
        break;

         case Phonon::PlayingState:
        qDebug("playing");
        loadingWidget->hide();
        videoWidget->show();
        break;

         case Phonon::StoppedState:
        qDebug("stopped");
        // play() has already been called when setting the source
        // but Phonon on Linux needs a little more help to start playback
        mediaObject->play();
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
    switch( mediaObject->state() ) {
    case Phonon::PlayingState:
        mediaObject->pause();
        break;
    default:
        mediaObject->play();
        break;
    }
}

void MediaView::fullscreen() {

#ifdef Q_WS_MAC
    splitterState = splitter->saveState();
    videoWidget->setParent(0);
    videoWidget->showFullScreen();
#else
    videoWidget->setFullScreen(!videoWidget->isFullScreen());
#endif

}

void MediaView::exitFullscreen() {

#ifdef Q_WS_MAC
    videoWidget->setParent(this);
    splitter->addWidget(videoWidget);
    videoWidget->showNormal();
    splitter->restoreState(splitterState);
#else
    videoWidget->setFullScreen(false);
#endif

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
    videoWidget->hide();
    loadingWidget->setVideo(video);
    loadingWidget->show();

    mediaObject->pause();

    connect(video, SIGNAL(gotStreamUrl(QUrl)), SLOT(gotStreamUrl(QUrl)));
    video->loadStreamUrl();

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
