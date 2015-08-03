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

#include "mediaview.h"
#include "playlistmodel.h"
#include "playlistview.h"
#include "loadingwidget.h"
#include "videoareawidget.h"
#include "networkaccess.h"
#include "minisplitter.h"
#include "constants.h"
#include "downloadmanager.h"
#include "downloaditem.h"
#include "mainwindow.h"
#include "temporary.h"
#include "refinesearchwidget.h"
#include "sidebarwidget.h"
#include "sidebarheader.h"
#ifdef APP_ACTIVATION
#include "activation.h"
#endif
#ifdef APP_EXTRA
#include "extra.h"
#endif
#include "videosource.h"
#include "ytsearch.h"
#include "searchparams.h"
#include "ytsinglevideosource.h"
#include "channelaggregator.h"
#include "iconutils.h"
#include "ytchannel.h"
#ifdef APP_SNAPSHOT
#include "snapshotsettings.h"
#endif
#include "datautils.h"
#include "compatibility/qurlqueryhelper.h"

namespace The {
NetworkAccess* http();
QHash<QString, QAction*>* globalActions();
QHash<QString, QMenu*>* globalMenus();
QNetworkAccessManager* networkAccessManager();
}

MediaView* MediaView::instance() {
    static MediaView *i = new MediaView();
    return i;
}

MediaView::MediaView(QWidget *parent) : QWidget(parent)
  , stopped(false)
  , downloadItem(0)
  #ifdef APP_SNAPSHOT
  , snapshotSettings(0)
  #endif
{ }

void MediaView::initialize() {
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    splitter = new MiniSplitter();

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
            MainWindow::instance(), SLOT(search(QString)));
    splitter->addWidget(sidebar);

    videoAreaWidget = new VideoAreaWidget(this);
    // videoAreaWidget->setMinimumSize(320,240);

#ifdef APP_PHONON
    videoWidget = new Phonon::VideoWidget(this);
    videoAreaWidget->setVideoWidget(videoWidget);
#endif
    videoAreaWidget->setListModel(playlistModel);

    loadingWidget = new LoadingWidget(this);
    videoAreaWidget->setLoadingWidget(loadingWidget);

    splitter->addWidget(videoAreaWidget);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 8);

    // restore splitter state
    QSettings settings;
    splitter->restoreState(settings.value("splitter").toByteArray());
    splitter->setChildrenCollapsible(false);
    connect(splitter, SIGNAL(splitterMoved(int,int)), SLOT(maybeAdjustWindowSize()));

    layout->addWidget(splitter);

    errorTimer = new QTimer(this);
    errorTimer->setSingleShot(true);
    errorTimer->setInterval(3000);
    connect(errorTimer, SIGNAL(timeout()), SLOT(skipVideo()));

#ifdef APP_ACTIVATION
    demoTimer = new QTimer(this);
    demoTimer->setSingleShot(true);
    connect(demoTimer, SIGNAL(timeout()), SLOT(demoMessage()));
#endif

    connect(videoAreaWidget, SIGNAL(doubleClicked()),
            The::globalActions()->value("fullscreen"), SLOT(trigger()));

    QAction* refineSearchAction = The::globalActions()->value("refine-search");
    connect(refineSearchAction, SIGNAL(toggled(bool)),
            sidebar, SLOT(toggleRefineSearch(bool)));

    currentVideoActions
            << The::globalActions()->value("webpage")
            << The::globalActions()->value("pagelink")
            << The::globalActions()->value("videolink")
            << The::globalActions()->value("open-in-browser")
           #ifdef APP_SNAPSHOT
            << The::globalActions()->value("snapshot")
           #endif
            << The::globalActions()->value("findVideoParts")
            << The::globalActions()->value("skip")
            << The::globalActions()->value("previous")
            << The::globalActions()->value("stopafterthis")
            << The::globalActions()->value("related-videos")
            << The::globalActions()->value("refine-search")
            << The::globalActions()->value("twitter")
            << The::globalActions()->value("facebook")
            << The::globalActions()->value("buffer")
            << The::globalActions()->value("email");

#ifndef APP_PHONON_SEEK
    QSlider *slider = MainWindow::instance()->getSlider();
    connect(slider, SIGNAL(valueChanged(int)), SLOT(sliderMoved(int)));
#endif
}

#ifdef APP_PHONON
void MediaView::setMediaObject(Phonon::MediaObject *mediaObject) {
    this->mediaObject = mediaObject;
    Phonon::createPath(mediaObject, videoWidget);
    connect(mediaObject, SIGNAL(finished()), SLOT(playbackFinished()));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(mediaObject, SIGNAL(aboutToFinish()), SLOT(aboutToFinish()));
}
#endif

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
                YTSingleVideoSource *singleVideoSource = new YTSingleVideoSource(this);
                singleVideoSource->setVideoId(videoId);
                setVideoSource(singleVideoSource);
                return;
            }
        }
    }
    YTSearch *ytSearch = new YTSearch(searchParams, this);
    ytSearch->setAsyncDetails(true);
    connect(ytSearch, SIGNAL(gotDetails()), playlistModel, SLOT(emitDataChanged()));
    setVideoSource(ytSearch);
}

void MediaView::setVideoSource(VideoSource *videoSource, bool addToHistory, bool back) {
    Q_UNUSED(back);
    stopped = false;

#ifdef APP_ACTIVATION
    demoTimer->stop();
#endif
    errorTimer->stop();

    // qDebug() << "Adding VideoSource" << videoSource->getName() << videoSource;

    if (addToHistory) {
        int currentIndex = getHistoryIndex();
        if (currentIndex >= 0 && currentIndex < history.size() - 1) {
            while (history.size() > currentIndex + 1) {
                VideoSource *vs = history.takeLast();
                if (!vs->parent()) {
                    qDebug() << "Deleting VideoSource" << vs->getName() << vs;
                    delete vs;
                }
            }
        }
        history.append(videoSource);
    }

#ifdef APP_EXTRA
    if (history.size() > 1)
        Extra::slideTransition(playlistView->viewport(), playlistView->viewport(), back);
#endif

    playlistModel->setVideoSource(videoSource);

    sidebar->showPlaylist();
    sidebar->getRefineSearchWidget()->setSearchParams(getSearchParams());
    sidebar->hideSuggestions();
    sidebar->getHeader()->updateInfo();

    SearchParams *searchParams = getSearchParams();
    bool isChannel = searchParams && !searchParams->channelId().isEmpty();
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
            setVideoSource(previousVideoSource, false, true);
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
    Video *currentVideo = playlistModel->activeVideo();
    if (currentVideo) {
        MainWindow::instance()->setWindowTitle(
                    currentVideo->title() + " - " + Constants::NAME);
    }

    // optimize window for 16:9 video
    QTimer::singleShot(50, this, SLOT(maybeAdjustWindowSize()));

    playlistView->setFocus();
}

void MediaView::disappear() {

}

void MediaView::handleError(const QString &message) {
    qWarning() << __PRETTY_FUNCTION__ << message;
#ifdef APP_PHONON_SEEK
    mediaObject->play();
#else
    QTimer::singleShot(500, this, SLOT(startPlaying()));
#endif
}

#ifdef APP_PHONON
void MediaView::stateChanged(Phonon::State newState, Phonon::State /*oldState*/) {
    if (newState == Phonon::PlayingState)
        videoAreaWidget->showVideo();
    else if (newState == Phonon::ErrorState) {
        qWarning() << "Phonon error:" << mediaObject->errorString() << mediaObject->errorType();
        if (mediaObject->errorType() == Phonon::FatalError)
            handleError(mediaObject->errorString());
    }
}
#endif

void MediaView::pause() {
#ifdef APP_PHONON
    switch( mediaObject->state() ) {
    case Phonon::PlayingState:
        mediaObject->pause();
        break;
    default:
        mediaObject->play();
        break;
    }
#endif
}

QRegExp MediaView::wordRE(const QString &s) {
    return QRegExp("\\W" + s + "\\W?", Qt::CaseInsensitive);
}

void MediaView::stop() {
    stopped = true;

    while (!history.isEmpty()) {
        VideoSource *videoSource = history.takeFirst();
        if (!videoSource->parent()) delete videoSource;
    }

    playlistModel->abortSearch();
    videoAreaWidget->clear();
    videoAreaWidget->update();
    errorTimer->stop();
    playlistView->selectionModel()->clearSelection();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
        downloadItem = 0;
        currentVideoSize = 0;
    }
    The::globalActions()->value("refine-search")->setChecked(false);
    updateSubscriptionAction(0, false);
#ifdef APP_ACTIVATION
    demoTimer->stop();
#endif

    foreach (QAction *action, currentVideoActions)
        action->setEnabled(false);

    QAction *a = The::globalActions()->value("download");
    a->setEnabled(false);
    a->setVisible(false);

#ifdef APP_PHONON
    mediaObject->stop();
#endif
    currentVideoId.clear();

#ifndef APP_PHONON_SEEK
    QSlider *slider = MainWindow::instance()->getSlider();
    slider->setEnabled(false);
    slider->setValue(0);
#endif

    if (snapshotSettings) {
        delete snapshotSettings;
        snapshotSettings = 0;
    }
}

const QString & MediaView::getCurrentVideoId() {
    return currentVideoId;
}

void MediaView::activeRowChanged(int row) {
    if (stopped) return;

    errorTimer->stop();

#ifdef APP_PHONON
    mediaObject->stop();
#endif
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
        downloadItem = 0;
        currentVideoSize = 0;
    }

    Video *video = playlistModel->videoAt(row);
    if (!video) return;

    videoAreaWidget->showLoading(video);

    connect(video, SIGNAL(gotStreamUrl(QUrl)),
            SLOT(gotStreamUrl(QUrl)), Qt::UniqueConnection);
    connect(video, SIGNAL(errorStreamUrl(QString)),
            SLOT(skip()), Qt::UniqueConnection);
    video->loadStreamUrl();

    // video title in titlebar
    MainWindow::instance()->setWindowTitle(video->title() + " - " + Constants::NAME);

    // ensure active item is visible
    if (row != -1) {
        QModelIndex index = playlistModel->index(row, 0, QModelIndex());
        playlistView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }

    // enable/disable actions
    The::globalActions()->value("download")->setEnabled(
                DownloadManager::instance()->itemForVideo(video) == 0);
    The::globalActions()->value("previous")->setEnabled(row > 0);
    The::globalActions()->value("stopafterthis")->setEnabled(true);
    The::globalActions()->value("related-videos")->setEnabled(true);

    bool enableDownload = video->license() == Video::LicenseCC;
#ifdef APP_ACTIVATION
    enableDownload = enableDownload || Activation::instance().isLegacy();
#endif
#ifdef APP_DOWNLOADS
    enableDownload = true;
#endif
    QAction *a = The::globalActions()->value("download");
    a->setEnabled(enableDownload);
    a->setVisible(enableDownload);

    updateSubscriptionAction(video, YTChannel::isSubscribed(video->channelId()));

    foreach (QAction *action, currentVideoActions)
        action->setEnabled(true);

#ifndef APP_PHONON_SEEK
    QSlider *slider = MainWindow::instance()->getSlider();
    slider->setEnabled(false);
    slider->setValue(0);
#endif

    if (snapshotSettings) {
        delete snapshotSettings;
        snapshotSettings = 0;
        MainWindow::instance()->adjustStatusBarVisibility();
    }

    // see you in gotStreamUrl...
}

void MediaView::gotStreamUrl(QUrl streamUrl) {
    if (stopped) return;
    if (!streamUrl.isValid()) {
        skip();
        return;
    }

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender in" << __PRETTY_FUNCTION__;
        return;
    }
    video->disconnect(this);

    currentVideoId = video->id();

#ifdef APP_PHONON_SEEK
    mediaObject->setCurrentSource(streamUrl);
    mediaObject->play();
#else
    startDownloading();
#endif

    // ensure we always have videos ahead
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

#ifdef APP_EXTRA
    Extra::notify(video->title(), video->channelTitle(), video->formattedDuration());
#endif

    ChannelAggregator::instance()->videoWatched(video);
}

void MediaView::downloadStatusChanged() {
    // qDebug() << __PRETTY_FUNCTION__;
    switch(downloadItem->status()) {
    case Downloading:
        // qDebug() << "Downloading";
        if (downloadItem->offset() == 0) startPlaying();
        else {
#ifdef APP_PHONON
            // qDebug() << "Seeking to" << downloadItem->offset();
            mediaObject->seek(offsetToTime(downloadItem->offset()));
            mediaObject->play();
#endif
        }
        break;
    case Starting:
        // qDebug() << "Starting";
        break;
    case Finished:
        // qDebug() << "Finished" << mediaObject->state();
#ifdef APP_PHONON_SEEK
        MainWindow::instance()->getSeekSlider()->setEnabled(mediaObject->isSeekable());
#endif
        break;
    case Failed:
        // qDebug() << "Failed";
        skip();
        break;
    case Idle:
        // qDebug() << "Idle";
        break;
    }
}

void MediaView::startPlaying() {
    // qDebug() << __PRETTY_FUNCTION__;
    if (stopped) return;
    if (!downloadItem) {
        skip();
        return;
    }

    if (downloadItem->offset() == 0) {
        currentVideoSize = downloadItem->bytesTotal();
        // qDebug() << "currentVideoSize" << currentVideoSize;
    }

    // go!
    QString source = downloadItem->currentFilename();
    qDebug() << "Playing" << source << QFile::exists(source);
#ifdef APP_PHONON
    mediaObject->setCurrentSource(QUrl::fromLocalFile(source));
    mediaObject->play();
#endif
#ifdef APP_PHONON_SEEK
    MainWindow::instance()->getSeekSlider()->setEnabled(false);
#else
    QSlider *slider = MainWindow::instance()->getSlider();
    slider->setEnabled(true);
#endif
}

void MediaView::itemActivated(const QModelIndex &index) {
    if (playlistModel->rowExists(index.row())) {

        // if it's the current video, just rewind and play
        Video *activeVideo = playlistModel->activeVideo();
        Video *video = playlistModel->videoAt(index.row());
        if (activeVideo && video && activeVideo == video) {
            // mediaObject->seek(0);
            sliderMoved(0);
#ifdef APP_PHONON
            mediaObject->play();
#endif
        } else playlistModel->setActiveRow(index.row());

        // the user doubleclicked on the "Search More" item
    } else {
        playlistModel->searchMore();
        playlistView->selectionModel()->clearSelection();
    }
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
#ifdef APP_PHONON
    qint64 currentTime = mediaObject->currentTime();
    qint64 totalTime = mediaObject->totalTime();
    qDebug() << __PRETTY_FUNCTION__ << currentTime << totalTime;
    if (totalTime < 1 || currentTime + 10000 < totalTime) {
        // QTimer::singleShot(500, this, SLOT(playbackResume()));
        mediaObject->seek(currentTime);
        mediaObject->play();
    }
#endif
}

void MediaView::playbackFinished() {
    if (stopped) return;

#ifdef APP_PHONON
    const qint64 totalTime = mediaObject->totalTime();
    const qint64 currentTime = mediaObject->currentTime();
    qDebug() << __PRETTY_FUNCTION__ << mediaObject->currentTime() << totalTime;
    // add 10 secs for imprecise Phonon backends (VLC, Xine)
    if (currentTime > 0 && currentTime + 10000 < totalTime) {
        // mediaObject->seek(currentTime);
        QTimer::singleShot(500, this, SLOT(playbackResume()));
    } else {
        QAction* stopAfterThisAction = The::globalActions()->value("stopafterthis");
        if (stopAfterThisAction->isChecked()) {
            stopAfterThisAction->setChecked(false);
        } else skip();
    }
#endif
}

void MediaView::playbackResume() {
    if (stopped) return;
#ifdef APP_PHONON
    const qint64 currentTime = mediaObject->currentTime();
    qDebug() << __PRETTY_FUNCTION__ << currentTime;
    if (currentTime > 0)
        mediaObject->seek(currentTime);
    mediaObject->play();
#endif
}

void MediaView::openWebPage() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
#ifdef APP_PHONON
    mediaObject->pause();
#endif
    QDesktopServices::openUrl(video->webpage());
}

void MediaView::copyWebPage() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QString address = video->webpage();
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

void MediaView::openInBrowser() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
#ifdef APP_PHONON
    mediaObject->pause();
#endif
    QDesktopServices::openUrl(video->getStreamUrl());
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

void MediaView::selectionChanged(const QItemSelection & /*selected*/,
                                 const QItemSelection & /*deselected*/) {
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
    playlistView->selectionModel()->setCurrentIndex(playlistModel->index(row>1?row:1),
                                                    QItemSelectionModel::NoUpdate);
}

void MediaView::moveDownSelected() {
    if (!playlistView->selectionModel()->hasSelection()) return;

    QModelIndexList indexes = playlistView->selectionModel()->selectedIndexes();
    qStableSort(indexes.begin(), indexes.end(), qGreater<QModelIndex>());
    playlistModel->move(indexes, false);

    // set current index after row moves to something more intuitive
    // (respect 1 static item on bottom)
    int row = indexes.first().row()+1, max = playlistModel->rowCount() - 2;
    playlistView->selectionModel()->setCurrentIndex(
                playlistModel->index(row>max?max:row), QItemSelectionModel::NoUpdate);
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
#ifdef APP_PHONON
    if (mediaObject->state() != Phonon::PlayingState) return;
    mediaObject->pause();
#endif

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
#ifdef APP_PHONON
        mediaObject->play();
#endif
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

#ifdef APP_SNAPSHOT
void MediaView::snapshot() {
    qint64 currentTime = mediaObject->currentTime() / 1000;

    QImage image = videoWidget->snapshot();
    if (image.isNull()) {
        qWarning() << "Null snapshot";
        return;
    }

    // QPixmap pixmap = QPixmap::grabWindow(videoWidget->winId());
    QPixmap pixmap = QPixmap::fromImage(image.scaled(videoWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    videoAreaWidget->showSnapshotPreview(pixmap);

    Video* video = playlistModel->activeVideo();
    if (!video) return;

    QString location = SnapshotSettings::getCurrentLocation();
    QDir dir(location);
    if (!dir.exists()) dir.mkpath(location);
    QString basename = video->title();
    QString format = video->duration() > 3600 ? "h_mm_ss" : "m_ss";
    basename += " (" + QTime().addSecs(currentTime).toString(format) + ")";
    basename = DataUtils::stringToFilename(basename);
    QString filename = location + "/" + basename + ".png";
    qDebug() << filename;
    image.save(filename, "PNG");

    if (snapshotSettings) delete snapshotSettings;
    snapshotSettings = new SnapshotSettings(videoWidget);
    snapshotSettings->setSnapshot(pixmap, filename);
    QStatusBar *statusBar = MainWindow::instance()->statusBar();
#ifdef APP_EXTRA
    Extra::fadeInWidget(statusBar, statusBar);
#endif
    statusBar->insertPermanentWidget(0, snapshotSettings);
    snapshotSettings->show();
    MainWindow::instance()->setStatusBarVisibility(true);
}
#endif

void MediaView::fullscreen() {
    videoAreaWidget->setParent(0);
    videoAreaWidget->showFullScreen();
}

void MediaView::startDownloading() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    Video *videoCopy = video->clone();
    if (downloadItem) {
        downloadItem->stop();
        delete downloadItem;
    }
    QString tempFile = Temporary::filename();
    downloadItem = new DownloadItem(videoCopy, video->getStreamUrl(), tempFile, this);
    connect(downloadItem, SIGNAL(statusChanged()),
            SLOT(downloadStatusChanged()), Qt::UniqueConnection);
    connect(downloadItem, SIGNAL(bufferProgress(int)),
            loadingWidget, SLOT(bufferStatus(int)), Qt::UniqueConnection);
    // connect(downloadItem, SIGNAL(finished()), SLOT(itemFinished()));
    connect(video, SIGNAL(errorStreamUrl(QString)),
            SLOT(handleError(QString)), Qt::UniqueConnection);
    connect(downloadItem, SIGNAL(error(QString)),
            SLOT(handleError(QString)), Qt::UniqueConnection);
    downloadItem->start();
}

void MediaView::maybeAdjustWindowSize() {
    QSettings settings;
    if (settings.value("adjustWindowSize", true).toBool())
        adjustWindowSize();
}

void MediaView::sliderMoved(int value) {
    Q_UNUSED(value);
#ifdef APP_PHONON
#ifndef APP_PHONON_SEEK

    if (currentVideoSize <= 0 || !downloadItem || !mediaObject->isSeekable())
        return;

    QSlider *slider = MainWindow::instance()->getSlider();
    if (slider->isSliderDown()) return;

    qint64 offset = (currentVideoSize * value) / slider->maximum();

    bool needsDownload = downloadItem->needsDownload(offset);
    if (needsDownload) {
        if (downloadItem->isBuffered(offset)) {
            qint64 realOffset = downloadItem->blankAtOffset(offset);
            if (offset < currentVideoSize)
                downloadItem->seekTo(realOffset, false);
            mediaObject->seek(offsetToTime(offset));
        } else {
            mediaObject->pause();
            downloadItem->seekTo(offset);
        }
    } else {
        // qDebug() << "simple seek";
        mediaObject->seek(offsetToTime(offset));
    }
#endif
#endif
}

qint64 MediaView::offsetToTime(qint64 offset) {
#ifdef APP_PHONON
    const qint64 totalTime = mediaObject->totalTime();
    return ((offset * totalTime) / currentVideoSize);
#endif
}

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
    query.remove(QRegExp(counterNumber + optionalSpace +
                         counterSeparators + optionalSpace + counterNumber));
    query.remove(wordRE("pr?t\\.?" + optionalSpace + counterNumber));
    query.remove(wordRE("ep\\.?" + optionalSpace + counterNumber));
    query.remove(wordRE("part" + optionalSpace + counterNumber));
    query.remove(wordRE("episode" + optionalSpace + counterNumber));
    query.remove(wordRE(tr("part", "This is for video parts, as in 'Cool video - part 1'") +
                        optionalSpace + counterNumber));
    query.remove(wordRE(tr("episode",
                           "This is for video parts, as in 'Cool series - episode 1'") +
                        optionalSpace + counterNumber));
    query.remove(QRegExp("[\\(\\)\\[\\]]"));

#define NUMBERS "one|two|three|four|five|six|seven|eight|nine|ten"

    QRegExp englishNumberRE = QRegExp(QLatin1String(".*(") + NUMBERS + ").*",
                                      Qt::CaseInsensitive);
    // bool numberAsWords = englishNumberRE.exactMatch(query);
    query.remove(englishNumberRE);

    QRegExp localizedNumberRE = QRegExp(QLatin1String(".*(") + tr(NUMBERS) + ").*",
                                        Qt::CaseInsensitive);
    // if (!numberAsWords) numberAsWords = localizedNumberRE.exactMatch(query);
    query.remove(localizedNumberRE);

    SearchParams *searchParams = new SearchParams();
    searchParams->setTransient(true);
    searchParams->setKeywords(query);
    searchParams->setChannelId(video->channelId());

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

void MediaView::relatedVideos() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    YTSingleVideoSource *singleVideoSource = new YTSingleVideoSource();
    singleVideoSource->setVideo(video->clone());
    singleVideoSource->setAsyncDetails(true);
    setVideoSource(singleVideoSource);
    The::globalActions()->value("related-videos")->setEnabled(false);
}

void MediaView::shareViaTwitter() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("https://twitter.com/intent/tweet");
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("via", "minitubeapp");
        urlHelper.addQueryItem("text", video->title());
        urlHelper.addQueryItem("url", video->webpage());
    }
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaFacebook() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("https://www.facebook.com/sharer.php");
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("t", video->title());
        urlHelper.addQueryItem("u", video->webpage());
    }
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaBuffer() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("http://bufferapp.com/add");
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("via", "minitubeapp");
        urlHelper.addQueryItem("text", video->title());
        urlHelper.addQueryItem("url", video->webpage());
        urlHelper.addQueryItem("picture", video->thumbnailUrl());
    }
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaEmail() {
    Video* video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("mailto:");
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("subject", video->title());
        const QString body = video->title() + "\n" +
                video->webpage() + "\n\n" +
                tr("Sent from %1").arg(Constants::NAME) + "\n" +
                Constants::WEBSITE;
        urlHelper.addQueryItem("body", body);
    }
    QDesktopServices::openUrl(url);
}

void MediaView::authorPushed(QModelIndex index) {
    Video* video = playlistModel->videoAt(index.row());
    if (!video) return;

    QString channelId = video->channelId();
    // if (channelId.isEmpty()) channelId = video->channelTitle();
    if (channelId.isEmpty()) return;

    SearchParams *searchParams = new SearchParams();
    searchParams->setChannelId(channelId);
    searchParams->setSortBy(SearchParams::SortByNewest);

    // go!
    search(searchParams);
}

void MediaView::updateSubscriptionAction(Video *video, bool subscribed) {
    QAction *subscribeAction = The::globalActions()->value("subscribe-channel");

    QString subscribeTip;
    QString subscribeText;
    if (!video) {
        subscribeText = subscribeAction->property("originalText").toString();
        subscribeAction->setEnabled(false);
    } else if (subscribed) {
        subscribeText = tr("Unsubscribe from %1").arg(video->channelTitle());
        subscribeTip = subscribeText;
        subscribeAction->setEnabled(true);
    } else {
        subscribeText = tr("Subscribe to %1").arg(video->channelTitle());
        subscribeTip = subscribeText;
        subscribeAction->setEnabled(true);
    }
    subscribeAction->setText(subscribeText);
    subscribeAction->setStatusTip(subscribeTip);

    if (subscribed) {
#ifdef APP_LINUX
        static QIcon tintedIcon;
        if (tintedIcon.isNull()) {
            QList<QSize> sizes;
            sizes << QSize(16, 16);
            tintedIcon = IconUtils::tintedIcon("bookmark-new", QColor(254, 240, 0), sizes);
        }
        subscribeAction->setIcon(tintedIcon);
#else
        subscribeAction->setIcon(IconUtils::icon("bookmark-remove"));
#endif
    } else {
        subscribeAction->setIcon(IconUtils::icon("bookmark-new"));
    }

    IconUtils::setupAction(subscribeAction);
}

void MediaView::toggleSubscription() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    QString userId = video->channelId();
    if (userId.isEmpty()) return;
    bool subscribed = YTChannel::isSubscribed(userId);
    if (subscribed) YTChannel::unsubscribe(userId);
    else YTChannel::subscribe(userId);
    updateSubscriptionAction(video, !subscribed);
}

void MediaView::adjustWindowSize() {
    if (!MainWindow::instance()->isMaximized() && !MainWindow::instance()->isFullScreen()) {
        const double ratio = 16. / 9.;
        const int w = videoAreaWidget->width();
        const int h = videoAreaWidget->height();
        const double currentVideoRatio = (double)w / (double)h;
        if (currentVideoRatio != ratio) {
            if (false && currentVideoRatio > ratio) {
                // we have vertical black bars
                int newWidth = (MainWindow::instance()->width() - w) + (h * ratio);
                MainWindow::instance()->resize(newWidth, MainWindow::instance()->height());
            } else {
                // horizontal black bars
                int newHeight = (MainWindow::instance()->height() - h) + (w / ratio);
                MainWindow::instance()->resize(MainWindow::instance()->width(), newHeight);
            }
        }
    }
}
