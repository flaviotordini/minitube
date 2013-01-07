#include "mediaview.h"
#include "playlistmodel.h"
#include "playlistview.h"
#include "loadingwidget.h"
#include "videoareawidget.h"
#include "networkaccess.h"
#include "videowidget.h"
#include "minisplitter.h"
#include "constants.h"
#include "downloadmanager.h"
#include "downloaditem.h"
#include "mainwindow.h"
#include "temporary.h"
#include "refinesearchwidget.h"
#include "sidebarwidget.h"
#include "sidebarheader.h"
#ifdef APP_MAC
#include "macfullscreen.h"
#include "macutils.h"
#endif
#ifdef APP_ACTIVATION
#include "activation.h"
#endif
#include "videosource.h"
#include "ytsearch.h"
#include "searchparams.h"
#include "ytsinglevideosource.h"

namespace The {
NetworkAccess* http();
QMap<QString, QAction*>* globalActions();
QMap<QString, QMenu*>* globalMenus();
QNetworkAccessManager* networkAccessManager();
}

MediaView* MediaView::instance() {
    static MediaView *i = new MediaView();
    return i;
}

MediaView::MediaView(QWidget *parent) : QWidget(parent) {
    reallyStopped = false;
    downloadItem = 0;

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    splitter = new MiniSplitter();
    splitter->setChildrenCollapsible(false);

    playlistView = new PlaylistView(this);
    // respond to the user doubleclicking a playlist item
    connect(playlistView, SIGNAL(activated(const QModelIndex &)),
            SLOT(itemActivated(const QModelIndex &)));

    playlistModel = new PlaylistModel();
    connect(playlistModel, SIGNAL(activeRowChanged(int)),
            SLOT(activeRowChanged(int)));
    // needed to restore the selection after dragndrop
    connect(playlistModel, SIGNAL(needSelectionFor(QList<Video*>)),
            SLOT(selectVideos(QList<Video*>)));
    playlistView->setModel(playlistModel);

    connect(playlistView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));

    connect(playlistView, SIGNAL(authorPushed(QModelIndex)), SLOT(authorPushed(QModelIndex)));

    sidebar = new SidebarWidget(this);
    sidebar->setPlaylist(playlistView);
    connect(sidebar->getRefineSearchWidget(), SIGNAL(searchRefined()),
            SLOT(searchAgain()));
    connect(playlistModel, SIGNAL(haveSuggestions(const QStringList &)),
            sidebar, SLOT(showSuggestions(const QStringList &)));
    connect(sidebar, SIGNAL(suggestionAccepted(QString)),
            MainWindow::instance(), SLOT(startToolbarSearch(QString)));
    splitter->addWidget(sidebar);

    videoAreaWidget = new VideoAreaWidget(this);
    videoAreaWidget->setMinimumSize(320,240);
    videoWidget = new Phonon::VideoWidget(this);
    videoAreaWidget->setVideoWidget(videoWidget);
    videoAreaWidget->setListModel(playlistModel);

    loadingWidget = new LoadingWidget(this);
    videoAreaWidget->setLoadingWidget(loadingWidget);

    splitter->addWidget(videoAreaWidget);

    layout->addWidget(splitter);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 6);

    // restore splitter state
    QSettings settings;
    splitter->restoreState(settings.value("splitter").toByteArray());

    errorTimer = new QTimer(this);
    errorTimer->setSingleShot(true);
    errorTimer->setInterval(3000);
    connect(errorTimer, SIGNAL(timeout()), SLOT(skipVideo()));

#ifdef APP_ACTIVATION
    demoTimer = new QTimer(this);
    demoTimer->setSingleShot(true);
    connect(demoTimer, SIGNAL(timeout()), SLOT(demoMessage()));
#endif

}

void MediaView::initialize() {
    connect(videoAreaWidget, SIGNAL(doubleClicked()),
            The::globalActions()->value("fullscreen"), SLOT(trigger()));

    QAction* refineSearchAction = The::globalActions()->value("refine-search");
    connect(refineSearchAction, SIGNAL(toggled(bool)),
            sidebar, SLOT(toggleRefineSearch(bool)));
}

void MediaView::setMediaObject(Phonon::MediaObject *mediaObject) {
    this->mediaObject = mediaObject;
    Phonon::createPath(mediaObject, videoWidget);
    connect(mediaObject, SIGNAL(finished()), SLOT(playbackFinished()));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            SLOT(currentSourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), SLOT(aboutToFinish()));
}

SearchParams* MediaView::getSearchParams() {
    VideoSource *videoSource = playlistModel->getVideoSource();
    if (videoSource && videoSource->metaObject()->className() == QLatin1String("YTSearch")) {
        YTSearch *search = dynamic_cast<YTSearch *>(videoSource);
        return search->getSearchParams();
    }
    return 0;
}

void MediaView::search(SearchParams *searchParams) {
    if (!searchParams->keywords().isEmpty()) {
        if (searchParams->keywords().startsWith("http://") ||
                searchParams->keywords().startsWith("https://")) {
            QString videoId = YTSearch::videoIdFromUrl(searchParams->keywords());
            if (!videoId.isEmpty()) {
                qDebug() << "single video";
                YTSingleVideoSource *singleVideoSource = new YTSingleVideoSource(this);
                singleVideoSource->setVideoId(videoId);
                setVideoSource(singleVideoSource);
                return;
            }
        }
    }
    setVideoSource(new YTSearch(searchParams, this));
}

void MediaView::setVideoSource(VideoSource *videoSource, bool addToHistory) {
    reallyStopped = false;

#ifdef APP_ACTIVATION
    demoTimer->stop();
#endif
    errorTimer->stop();

    if (addToHistory) {
        int currentIndex = getHistoryIndex();
        if (currentIndex >= 0 && currentIndex < history.size() - 1) {
            for (int i = currentIndex + 1; i < history.size(); i++) {
                VideoSource *vs = history.takeAt(i);
                if (!vs->parent()) delete vs;
            }
        }
        history.append(videoSource);
    }

    playlistModel->setVideoSource(videoSource);

    sidebar->showPlaylist();
    sidebar->getRefineSearchWidget()->setSearchParams(getSearchParams());
    sidebar->hideSuggestions();
    sidebar->getHeader()->updateInfo();

    SearchParams *searchParams = getSearchParams();
    bool isChannel = searchParams && !searchParams->author().isEmpty();
    playlistView->setClickableAuthors(!isChannel);
}

void MediaView::searchAgain() {
    VideoSource *currentVideoSource = playlistModel->getVideoSource();
    setVideoSource(currentVideoSource, false);
}

bool MediaView::canGoBack() {
    return getHistoryIndex() > 0;
}

void MediaView::goBack() {
    if (history.size() > 1) {
        int currentIndex = getHistoryIndex();
        if (currentIndex > 0) {
            VideoSource *previousVideoSource = history.at(currentIndex - 1);
            setVideoSource(previousVideoSource, false);
        }
    }
}

bool MediaView::canGoForward() {
    int currentIndex = getHistoryIndex();
    return currentIndex >= 0 && currentIndex < history.size() - 1;
}

void MediaView::goForward() {
    if (canGoForward()) {
        int currentIndex = getHistoryIndex();
        VideoSource *nextVideoSource = history.at(currentIndex + 1);
        setVideoSource(nextVideoSource, false);
    }
}

int MediaView::getHistoryIndex() {
    return history.lastIndexOf(playlistModel->getVideoSource());
}

void MediaView::appear() {
    playlistView->setFocus();
}

void MediaView::disappear() {

}

void MediaView::handleError(QString /* message */) {
    QTimer::singleShot(500, this, SLOT(startPlaying()));
}

void MediaView::stateChanged(Phonon::State newState, Phonon::State /*oldState*/) {
    if (newState == Phonon::PlayingState)
        videoAreaWidget->showVideo();
    else if (newState == Phonon::ErrorState) {
        qDebug() << "Phonon error:" << mediaObject->errorString() << mediaObject->errorType();
        if (mediaObject->errorType() == Phonon::FatalError)
            handleError(mediaObject->errorString());
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

QRegExp MediaView::wordRE(QString s) {
    return QRegExp("\\W" + s + "\\W?", Qt::CaseInsensitive);
}

void MediaView::stop() {
    playlistModel->abortSearch();
    reallyStopped = true;
    mediaObject->stop();
    videoAreaWidget->clear();
    errorTimer->stop();
    playlistView->selectionModel()->clearSelection();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
        downloadItem = 0;
    }
    The::globalActions()->value("refine-search")->setChecked(false);

    while (!history.isEmpty()) {
        VideoSource *videoSource = history.takeFirst();
        if (!videoSource->parent()) delete videoSource;
    }
}

void MediaView::activeRowChanged(int row) {
    if (reallyStopped) return;

    Video *video = playlistModel->videoAt(row);
    if (!video) return;

    // Related videos without video interruption
    if (row == 0) {
        VideoSource *videoSource = playlistModel->getVideoSource();
        if (videoSource && !history.isEmpty() &&
                mediaObject->state() == Phonon::PlayingState &&
                videoSource->metaObject()->className() == QLatin1String("YTSingleVideoSource")) {
            if (playlistModel->videoAt(row)->title() == downloadItem->getVideo()->title())
                return;
        }
    }

    errorTimer->stop();

    videoAreaWidget->showLoading(video);

    mediaObject->stop();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
        downloadItem = 0;
    }

    connect(video, SIGNAL(gotStreamUrl(QUrl)),
            SLOT(gotStreamUrl(QUrl)), Qt::UniqueConnection);
    connect(video, SIGNAL(errorStreamUrl(QString)),
            SLOT(handleError(QString)), Qt::UniqueConnection);

    video->loadStreamUrl();

    // video title in the statusbar
    MainWindow::instance()->showMessage(video->title());

    // ensure active item is visible
    // int row = listModel->activeRow();
    if (row != -1) {
        QModelIndex index = playlistModel->index(row, 0, QModelIndex());
        playlistView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }

    // enable/disable actions
    The::globalActions()->value("download")->setEnabled(DownloadManager::instance()->itemForVideo(video) == 0);
    The::globalActions()->value("skip")->setEnabled(true);
    The::globalActions()->value("previous")->setEnabled(row > 0);
    The::globalActions()->value("stopafterthis")->setEnabled(true);

    // see you in gotStreamUrl...

}

void MediaView::gotStreamUrl(QUrl streamUrl) {
    if (reallyStopped) return;

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender in" << __PRETTY_FUNCTION__;
        return;
    }
    video->disconnect(this);

    QString tempFile = Temporary::filename();

    Video *videoCopy = video->clone();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
    }
    downloadItem = new DownloadItem(videoCopy, streamUrl, tempFile, this);
    connect(downloadItem, SIGNAL(statusChanged()), SLOT(downloadStatusChanged()), Qt::UniqueConnection);
    // connect(downloadItem, SIGNAL(progress(int)), SLOT(downloadProgress(int)));
    connect(downloadItem, SIGNAL(bufferProgress(int)), loadingWidget, SLOT(bufferStatus(int)), Qt::UniqueConnection);
    // connect(downloadItem, SIGNAL(finished()), SLOT(itemFinished()));
    connect(video, SIGNAL(errorStreamUrl(QString)), SLOT(handleError(QString)), Qt::UniqueConnection);
    connect(downloadItem, SIGNAL(error(QString)), SLOT(handleError(QString)), Qt::UniqueConnection);
    downloadItem->start();

#ifdef Q_WS_MAC
    if (mac::canNotify())
        mac::notify(video->title(), video->author(), video->formattedDuration());
#endif
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
#ifdef Q_WS_X11
        MainWindow::instance()->getSeekSlider()->setEnabled(mediaObject->isSeekable());
#endif
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
    if (!downloadItem) {
        skip();
        return;
    }

    // go!
    QString source = downloadItem->currentFilename();
    qDebug() << "Playing" << source;
    mediaObject->setCurrentSource(source);
    mediaObject->play();
#ifdef Q_WS_X11
    MainWindow::instance()->getSeekSlider()->setEnabled(false);
#endif

    // ensure we always have 10 videos ahead
    playlistModel->searchNeeded();

    // ensure active item is visible
    int row = playlistModel->activeRow();
    if (row != -1) {
        QModelIndex index = playlistModel->index(row, 0, QModelIndex());
        playlistView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }

#ifdef APP_ACTIVATION
    if (!Activation::instance().isActivated())
        demoTimer->start(180000);
#endif

}

void MediaView::itemActivated(const QModelIndex &index) {
    if (playlistModel->rowExists(index.row())) {

        // if it's the current video, just rewind and play
        Video *activeVideo = playlistModel->activeVideo();
        Video *video = playlistModel->videoAt(index.row());
        if (activeVideo && video && activeVideo == video) {
            mediaObject->seek(0);
            mediaObject->play();
        } else playlistModel->setActiveRow(index.row());

        // the user doubleclicked on the "Search More" item
    } else {
        playlistModel->searchMore();
        playlistView->selectionModel()->clearSelection();
    }
}

void MediaView::currentSourceChanged(const Phonon::MediaSource /* source */ ) {

}

void MediaView::skipVideo() {
    // skippedVideo is useful for DELAYED skip operations
    // in order to be sure that we're skipping the video we wanted
    // and not another one
    if (skippedVideo) {
        if (playlistModel->activeVideo() != skippedVideo) {
            qDebug() << "Skip of video canceled";
            return;
        }
        int nextRow = playlistModel->rowForVideo(skippedVideo);
        nextRow++;
        if (nextRow == -1) return;
        playlistModel->setActiveRow(nextRow);
    }
}

void MediaView::skip() {
    int nextRow = playlistModel->nextRow();
    if (nextRow == -1) return;
    playlistModel->setActiveRow(nextRow);
}

void MediaView::skipBackward() {
    int prevRow = playlistModel->previousRow();
    if (prevRow == -1) return;
    playlistModel->setActiveRow(prevRow);
}

void MediaView::aboutToFinish() {
    qint64 currentTime = mediaObject->currentTime();
    qDebug() << __PRETTY_FUNCTION__ << currentTime;
    if (currentTime + 10000 < mediaObject->totalTime()) {
        // mediaObject->seek(mediaObject->currentTime());
        // QTimer::singleShot(500, this, SLOT(playbackResume()));
        mediaObject->seek(currentTime);
        mediaObject->play();
    }
}

void MediaView::playbackFinished() {
    qDebug() << __PRETTY_FUNCTION__ << mediaObject->currentTime();
    // qDebug() << "finished" << mediaObject->currentTime() << mediaObject->totalTime();
    // add 10 secs for imprecise Phonon backends (VLC, Xine)
    if (mediaObject->currentTime() + 10000 < mediaObject->totalTime()) {
        // mediaObject->seek(mediaObject->currentTime());
        QTimer::singleShot(500, this, SLOT(playbackResume()));
    } else {
        QAction* stopAfterThisAction = The::globalActions()->value("stopafterthis");
        if (stopAfterThisAction->isChecked()) {
            stopAfterThisAction->setChecked(false);
        } else skip();
    }
}

void MediaView::playbackResume() {
    qDebug() << __PRETTY_FUNCTION__ << mediaObject->currentTime();
    mediaObject->seek(mediaObject->currentTime());
    mediaObject->play();
}

void MediaView::openWebPage() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    mediaObject->pause();
    QDesktopServices::openUrl(video->webpage());
}

void MediaView::copyWebPage() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QString address = video->webpage().toString();
    QApplication::clipboard()->setText(address);
    QString message = tr("You can now paste the YouTube link into another application");
    MainWindow::instance()->showMessage(message);
}

void MediaView::copyVideoLink() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QApplication::clipboard()->setText(video->getStreamUrl().toEncoded());
    QString message = tr("You can now paste the video stream URL into another application")
            + ". " + tr("The link will be valid only for a limited time.");
    MainWindow::instance()->showMessage(message);
}

void MediaView::removeSelected() {
    if (!playlistView->selectionModel()->hasSelection()) return;
    QModelIndexList indexes = playlistView->selectionModel()->selectedIndexes();
    playlistModel->removeIndexes(indexes);
}

void MediaView::selectVideos(QList<Video*> videos) {
    foreach (Video *video, videos) {
        QModelIndex index = playlistModel->indexForVideo(video);
        playlistView->selectionModel()->select(index, QItemSelectionModel::Select);
        playlistView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }
}

void MediaView::selectionChanged(const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/) {
    const bool gotSelection = playlistView->selectionModel()->hasSelection();
    The::globalActions()->value("remove")->setEnabled(gotSelection);
    The::globalActions()->value("moveUp")->setEnabled(gotSelection);
    The::globalActions()->value("moveDown")->setEnabled(gotSelection);
}

void MediaView::moveUpSelected() {
    if (!playlistView->selectionModel()->hasSelection()) return;

    QModelIndexList indexes = playlistView->selectionModel()->selectedIndexes();
    qStableSort(indexes.begin(), indexes.end());
    playlistModel->move(indexes, true);

    // set current index after row moves to something more intuitive
    int row = indexes.first().row();
    playlistView->selectionModel()->setCurrentIndex(playlistModel->index(row>1?row:1), QItemSelectionModel::NoUpdate);
}

void MediaView::moveDownSelected() {
    if (!playlistView->selectionModel()->hasSelection()) return;

    QModelIndexList indexes = playlistView->selectionModel()->selectedIndexes();
    qStableSort(indexes.begin(), indexes.end(), qGreater<QModelIndex>());
    playlistModel->move(indexes, false);

    // set current index after row moves to something more intuitive (respect 1 static item on bottom)
    int row = indexes.first().row()+1, max = playlistModel->rowCount() - 2;
    playlistView->selectionModel()->setCurrentIndex(playlistModel->index(row>max?max:row), QItemSelectionModel::NoUpdate);
}

void MediaView::setPlaylistVisible(bool visible) {
    if (splitter->widget(0)->isVisible() == visible) return;
    splitter->widget(0)->setVisible(visible);
    playlistView->setFocus();
}

bool MediaView::isPlaylistVisible() {
    return splitter->widget(0)->isVisible();
}

void MediaView::saveSplitterState() {
    QSettings settings;
    settings.setValue("splitter", splitter->saveState());
}

#ifdef APP_ACTIVATION

static QPushButton *continueButton;

void MediaView::demoMessage() {
    if (mediaObject->state() != Phonon::PlayingState) return;
    mediaObject->pause();

    QMessageBox msgBox(this);
    msgBox.setIconPixmap(QPixmap(":/images/app.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setText(tr("This is just the demo version of %1.").arg(Constants::NAME));
    msgBox.setInformativeText(tr("It allows you to test the application and see if it works for you."));
    msgBox.setModal(true);
    // make it a "sheet" on the Mac
    msgBox.setWindowModality(Qt::WindowModal);

    continueButton = msgBox.addButton("5", QMessageBox::RejectRole);
    continueButton->setEnabled(false);
    QPushButton *buyButton = msgBox.addButton(tr("Get the full version"), QMessageBox::ActionRole);

    QTimeLine *timeLine = new QTimeLine(6000, this);
    timeLine->setCurveShape(QTimeLine::LinearCurve);
    timeLine->setFrameRange(5, 0);
    connect(timeLine, SIGNAL(frameChanged(int)), SLOT(updateContinueButton(int)));
    timeLine->start();

    msgBox.exec();

    if (msgBox.clickedButton() == buyButton) {
        MainWindow::instance()->showActivationView();
    } else {
        mediaObject->play();
        demoTimer->start(600000);
    }

    delete timeLine;

}

void MediaView::updateContinueButton(int value) {
    if (value == 0) {
        continueButton->setText(tr("Continue"));
        continueButton->setEnabled(true);
    } else {
        continueButton->setText(QString::number(value));
    }
}

#endif

void MediaView::downloadVideo() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    DownloadManager::instance()->addItem(video);
    The::globalActions()->value("downloads")->setVisible(true);
    QString message = tr("Downloading %1").arg(video->title());
    MainWindow::instance()->showMessage(message);
}

void MediaView::snapshot() {
    QImage image = videoWidget->snapshot();
    qDebug() << image.size();

    const QPixmap& pixmap = QPixmap::grabWindow(videoWidget->winId());
    // qDebug() << pixmap.size();
    videoAreaWidget->showSnapshotPreview(pixmap);
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

void MediaView::findVideoParts() {

    // parts
    Video* video = playlistModel->activeVideo();
    if (!video) return;

    QString query = video->title();

    static QString optionalSpace = "\\s*";
    static QString staticCounterSeparators = "[\\/\\-]";
    QString counterSeparators = "( of | " +
            tr("of", "Used in video parts, as in '2 of 3'") +
            " |" + staticCounterSeparators + ")";

    // numbers from 1 to 15
    static QString counterNumber = "([1-9]|1[0-5])";

    // query.remove(QRegExp(counterSeparators + optionalSpace + counterNumber));
    query.remove(QRegExp(counterNumber + optionalSpace + counterSeparators + optionalSpace + counterNumber));
    query.remove(wordRE("pr?t\\.?" + optionalSpace + counterNumber));
    query.remove(wordRE("ep\\.?" + optionalSpace + counterNumber));
    query.remove(wordRE("part" + optionalSpace + counterNumber));
    query.remove(wordRE("episode" + optionalSpace + counterNumber));
    query.remove(wordRE(tr("part", "This is for video parts, as in 'Cool video - part 1'") +
                        optionalSpace + counterNumber));
    query.remove(wordRE(tr("episode", "This is for video parts, as in 'Cool series - episode 1'") +
                        optionalSpace + counterNumber));
    query.remove(QRegExp("[\\(\\)\\[\\]]"));

#define NUMBERS "one|two|three|four|five|six|seven|eight|nine|ten"

    QRegExp englishNumberRE = QRegExp(QLatin1String(".*(") + NUMBERS + ").*", Qt::CaseInsensitive);
    // bool numberAsWords = englishNumberRE.exactMatch(query);
    query.remove(englishNumberRE);

    QRegExp localizedNumberRE = QRegExp(QLatin1String(".*(") + tr(NUMBERS) + ").*", Qt::CaseInsensitive);
    // if (!numberAsWords) numberAsWords = localizedNumberRE.exactMatch(query);
    query.remove(localizedNumberRE);

    SearchParams *searchParams = new SearchParams();
    searchParams->setTransient(true);
    searchParams->setKeywords(query);
    searchParams->setAuthor(video->author());

    /*
    if (!numberAsWords) {
        qDebug() << "We don't have number as words";
        // searchParams->setSortBy(SearchParams::SortByNewest);
        // TODO searchParams->setReverseOrder(true);
        // TODO searchParams->setMax(50);
    }
    */

    search(searchParams);

}

void MediaView::shareViaTwitter() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("https://twitter.com/intent/tweet");
    url.addQueryItem("via", "minitubeapp");
    url.addQueryItem("text", video->title());
    url.addQueryItem("url", video->webpage().toString());
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaFacebook() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("https://www.facebook.com/sharer.php");
    url.addQueryItem("t", video->title());
    url.addQueryItem("u", video->webpage().toString());
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaBuffer() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("http://bufferapp.com/add");
    url.addQueryItem("via", "minitubeapp");
    url.addQueryItem("text", video->title());
    url.addQueryItem("url", video->webpage().toString());
    url.addQueryItem("picture", video->thumbnailUrl());
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaEmail() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("mailto:");
    url.addQueryItem("subject", video->title());
    QString body = video->title() + "\n" +
            video->webpage().toString() + "\n\n" +
            tr("Sent from %1").arg(Constants::NAME) + "\n" +
            Constants::WEBSITE;
    url.addQueryItem("body", body);
    QDesktopServices::openUrl(url);
}

void MediaView::authorPushed(QModelIndex index) {
    Video* video = playlistModel->videoAt(index.row());
    if (!video) return;

    QString channel = video->authorUri();
    if (channel.isEmpty()) channel = video->author();
    if (channel.isEmpty()) return;

    SearchParams *searchParams = new SearchParams();
    searchParams->setAuthor(channel);
    searchParams->setSortBy(SearchParams::SortByNewest);

    // go!
    search(searchParams);
}
