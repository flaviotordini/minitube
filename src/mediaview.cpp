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
#include "constants.h"
#include "downloadmanager.h"
#include "http.h"
#include "loadingwidget.h"
#include "mainwindow.h"
#include "minisplitter.h"
#include "playlistmodel.h"
#include "playlistview.h"
#include "refinesearchwidget.h"
#include "sidebarheader.h"
#include "sidebarwidget.h"
#include "temporary.h"
#include "videoarea.h"
#ifdef APP_ACTIVATION
#include "activation.h"
#include "activationview.h"
#endif
#ifdef APP_EXTRA
#include "extra.h"
#endif
#include "channelaggregator.h"
#include "iconutils.h"
#include "searchparams.h"
#include "videosource.h"
#include "ytchannel.h"
#include "ytsearch.h"
#ifdef APP_SNAPSHOT
#include "snapshotsettings.h"
#endif
#include "datautils.h"
#include "idle.h"
#include "videodefinition.h"

#include "searchvideosource.h"
#include "singlevideosource.h"
#include "videoapi.h"

MediaView *MediaView::instance() {
    static MediaView *i = new MediaView();
    return i;
}

MediaView::MediaView(QWidget *parent)
    : View(parent), splitter(nullptr), stopped(false)
#ifdef APP_SNAPSHOT
      ,
      snapshotSettings(nullptr)
#endif
      ,
      pauseTime(0) {
}

void MediaView::initialize() {
    MainWindow *mainWindow = MainWindow::instance();

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    splitter = new MiniSplitter();
    layout->addWidget(splitter);

    playlistView = new PlaylistView();
    playlistView->setParent(this);
    connect(playlistView, SIGNAL(activated(const QModelIndex &)),
            SLOT(onItemActivated(const QModelIndex &)));

    playlistModel = new PlaylistModel();
    connect(playlistModel, &PlaylistModel::activeVideoChanged, this,
            &MediaView::activeVideoChanged);
    // needed to restore the selection after dragndrop
    connect(playlistModel, SIGNAL(needSelectionFor(QVector<Video *>)),
            SLOT(selectVideos(QVector<Video *>)));
    playlistView->setModel(playlistModel);

    connect(playlistView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));

    connect(playlistView, SIGNAL(authorPushed(QModelIndex)), SLOT(onAuthorPushed(QModelIndex)));

    sidebar = new SidebarWidget(this);
    sidebar->setPlaylist(playlistView);
    sidebar->setMaximumWidth(playlistView->minimumWidth() * 3);
    connect(sidebar->getRefineSearchWidget(), SIGNAL(searchRefined()), SLOT(searchAgain()));
    connect(playlistModel, SIGNAL(haveSuggestions(const QStringList &)), sidebar,
            SLOT(showSuggestions(const QStringList &)));
    connect(sidebar, SIGNAL(suggestionAccepted(QString)), mainWindow, SLOT(search(QString)));
    splitter->addWidget(sidebar);

    videoAreaWidget = new VideoArea(this);
    videoAreaWidget->setListModel(playlistModel);

    loadingWidget = new LoadingWidget(this);
    videoAreaWidget->setLoadingWidget(loadingWidget);

    splitter->addWidget(videoAreaWidget);

    // restore splitter state
    QSettings settings;
    if (settings.contains("splitter"))
        splitter->restoreState(settings.value("splitter").toByteArray());
    else {
        int sidebarDefaultWidth = 180;
        splitter->setSizes(QList<int>() << sidebarDefaultWidth
                                        << splitter->size().width() - sidebarDefaultWidth);
    }
    splitter->setChildrenCollapsible(false);
    connect(splitter, SIGNAL(splitterMoved(int, int)), SLOT(adjustWindowSize()));

    errorTimer = new QTimer(this);
    errorTimer->setSingleShot(true);
    errorTimer->setInterval(3000);
    connect(errorTimer, SIGNAL(timeout()), SLOT(skipVideo()));

#ifdef APP_ACTIVATION
    demoTimer = new QTimer(this);
    demoTimer->setSingleShot(true);
    connect(
            demoTimer, &QTimer::timeout, this,
            [this] {
                if (media->state() != Media::PlayingState) return;
                media->pause();
                connect(
                        ActivationView::instance(), &ActivationView::done, media,
                        [this] { media->play(); }, Qt::UniqueConnection);
                MainWindow::instance()->showActivationView();
            },
            Qt::QueuedConnection);
#endif

    connect(videoAreaWidget, SIGNAL(doubleClicked()), mainWindow->getAction("fullscreen"),
            SLOT(trigger()));

    QAction *refineSearchAction = mainWindow->getAction("refineSearch");
    connect(refineSearchAction, SIGNAL(toggled(bool)), sidebar, SLOT(toggleRefineSearch(bool)));

    const QVector<const char *> videoActionNames = {
#ifdef APP_SNAPSHOT
            "snapshot",
#endif
            "webpage",  "pagelink", "videolink",     "openInBrowser", "findVideoParts",
            "skip",     "previous", "stopafterthis", "relatedVideos", "refineSearch",
            "twitter",  "facebook", "email",
            "systrayplaypause", "systrayprevious", "systraynext"};
    currentVideoActions.reserve(videoActionNames.size());
    for (auto *name : videoActionNames) {
        currentVideoActions.append(mainWindow->getAction(name));
    }

    for (int i = 0; i < 10; ++i) {
        QAction *action = new QAction(QString());
        action->setShortcut(Qt::Key_0 + i);
        action->setAutoRepeat(false);
        connect(action, &QAction::triggered, this, [this, i] {
            qint64 duration = media->duration();
            // dur : pos = 100 : i*10
            qint64 position = (duration * (i * 10)) / 100;
            media->seek(position);
        });
        addAction(action);
        playingVideoActions << action;
    }

    QAction *leftAction = new QAction(tr("Rewind %1 seconds").arg(10));
    leftAction->setShortcut(Qt::Key_Left);
    leftAction->setAutoRepeat(false);
    connect(leftAction, &QAction::triggered, this, [this] { media->relativeSeek(-10000); });
    addAction(leftAction);
    playingVideoActions << leftAction;

    QAction *rightAction = new QAction(tr("Fast forward %1 seconds").arg(10));
    rightAction->setShortcut(Qt::Key_Right);
    rightAction->setAutoRepeat(false);
    connect(rightAction, &QAction::triggered, this, [this] { media->relativeSeek(10000); });
    addAction(rightAction);
    playingVideoActions << rightAction;
}

void MediaView::setMedia(Media *media) {
    this->media = media;

    videoWidget = media->videoWidget();
    videoAreaWidget->setVideoWidget(videoWidget);

    connect(media, &Media::finished, this, &MediaView::onPlaybackFinished);
    connect(media, &Media::stateChanged, this, &MediaView::mediaStateChanged);
    connect(media, &Media::aboutToFinish, this, &MediaView::onAboutToFinish);
    connect(media, &Media::bufferStatus, loadingWidget, &LoadingWidget::bufferStatus);
}

SearchParams *MediaView::getSearchParams() {
    VideoSource *videoSource = playlistModel->getVideoSource();
    if (!videoSource) return nullptr;
    auto clazz = videoSource->metaObject()->className();
    if (clazz == QLatin1String("SearchVideoSource")) {
        auto search = qobject_cast<SearchVideoSource *>(videoSource);
        if (search) return search->getSearchParams();
    }
    return nullptr;
}

void MediaView::search(SearchParams *searchParams) {
    if (!searchParams->keywords().isEmpty()) {
        if (searchParams->keywords().startsWith("http://") ||
            searchParams->keywords().startsWith("https://")) {
            QString videoId = YTSearch::videoIdFromUrl(searchParams->keywords());
            if (!videoId.isEmpty()) {
                auto source = new SingleVideoSource(this);
                source->setVideoId(videoId);
                setVideoSource(source);

                QTime tstamp = YTSearch::videoTimestampFromUrl(searchParams->keywords());
                pauseTime = QTime(0, 0).msecsTo(tstamp);
                return;
            }
        }
    }

    VideoSource *search = new SearchVideoSource(searchParams);
    setVideoSource(search);
}

void MediaView::setVideoSource(VideoSource *videoSource, bool addToHistory, bool back) {
    Q_UNUSED(back);
    stopped = false;
    errorTimer->stop();

    // qDebug() << "Adding VideoSource" << videoSource->getName() << videoSource;

    if (addToHistory) {
        int currentIndex = getHistoryIndex();
        if (currentIndex >= 0 && currentIndex < history.size() - 1) {
            while (history.size() > currentIndex + 1) {
                VideoSource *vs = history.takeLast();
                if (!vs->parent()) {
                    qDebug() << "Deleting VideoSource" << vs->getName() << vs;
                    vs->abort();
                    vs->deleteLater();
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

    if (media->state() == Media::StoppedState) {
        QSettings settings;
        if (settings.value("manualplay", false).toBool()) {
            videoAreaWidget->showPickMessage();
        }
    }

    SearchParams *searchParams = getSearchParams();

    sidebar->showPlaylist();
    sidebar->getRefineSearchWidget()->setSearchParams(searchParams);
    sidebar->hideSuggestions();
    sidebar->getHeader()->updateInfo();

    bool isChannel = searchParams && !searchParams->channelId().isEmpty();
    if (isChannel) {
        updateSubscriptionActionForChannel(searchParams->channelId());
    }
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
    MainWindow::instance()->showToolbar();

    Video *currentVideo = playlistModel->activeVideo();
    if (currentVideo) {
        MainWindow::instance()->setWindowTitle(currentVideo->getTitle() + " - " + Constants::NAME);
        MainWindow::instance()->setToolTip(tr("Playing: %1").arg(currentVideo->getTitle()));
    }

    playlistView->setFocus();
}

void MediaView::disappear() {
    MainWindow::instance()->hideToolbar();
}

void MediaView::handleError(const QString &message) {
    qWarning() << __PRETTY_FUNCTION__ << message;
#ifndef QT_NO_DEBUG_OUTPUT
    MainWindow::instance()->showMessage(message);
#endif
}

void MediaView::mediaStateChanged(Media::State state) {
    if (pauseTime > 0 && (state == Media::PlayingState || state == Media::BufferingState)) {
        qDebug() << "Seeking to" << pauseTime;
        media->seek(pauseTime);
        media->play();
        pauseTime = 0;
    }
    if (state == Media::PlayingState) {
        videoAreaWidget->showVideo();
    } else if (state == Media::ErrorState) {
        handleError(media->errorString());
    }

    bool enablePlayingVideoActions = state != Media::StoppedState;
    for (QAction *action : qAsConst(playingVideoActions))
        action->setEnabled(enablePlayingVideoActions);

    if (state == Media::PlayingState) {
        bool res = Idle::preventDisplaySleep(QString("%1 is playing").arg(Constants::NAME));
        if (!res) qWarning() << "Error disabling idle display sleep" << Idle::displayErrorMessage();
    } else if (state == Media::PausedState || state == Media::StoppedState) {
        bool res = Idle::allowDisplaySleep();
        if (!res) qWarning() << "Error enabling idle display sleep" << Idle::displayErrorMessage();
    }
}

void MediaView::pause() {
	 Video *activeVideo = playlistModel->activeVideo();

    switch (media->state()) {
    case Media::PlayingState:
        qDebug() << "Pausing";
        media->pause();
        pauseTimer.start();
        MainWindow::instance()->setToolTip(tr("Paused: %1").arg(activeVideo->getTitle()));
        break;
    default:
        if (pauseTimer.isValid() && pauseTimer.hasExpired(60000)) {
            qDebug() << "Pause timer expired";
            pauseTimer.invalidate();

            if (activeVideo) {
                connect(activeVideo, &Video::gotStreamUrl, this,
                        &MediaView::resumeWithNewStreamUrl);
                activeVideo->loadStreamUrl();
            } else
                qDebug() << "No active video";

        } else {
            qDebug() << "Playing" << media->file();
            media->play();

            MainWindow::instance()->setToolTip(tr("Playing: %1").arg(activeVideo->getTitle()));
        }
        break;
    }
}

QRegularExpression MediaView::wordRE(const QString &s) {
    return QRegularExpression("\\W" + s + "\\W?", QRegularExpression::CaseInsensitiveOption);
}

void MediaView::stop() {
    stopped = true;

    while (!history.isEmpty()) {
        VideoSource *videoSource = history.takeFirst();
        // Don't delete videoSource in the Browse view
        if (!videoSource->parent()) {
            videoSource->abort();
            videoSource->deleteLater();
        }
    }

    playlistModel->abortSearch();
    videoAreaWidget->clear();
    videoAreaWidget->update();
    errorTimer->stop();
    playlistView->selectionModel()->clearSelection();

    MainWindow::instance()->getAction("refineSearch")->setChecked(false);
    updateSubscriptionActionForVideo(nullptr, false);
#ifdef APP_ACTIVATION
    demoTimer->stop();
#endif

    for (QAction *action : qAsConst(currentVideoActions))
        action->setEnabled(false);

    QAction *a = MainWindow::instance()->getAction("download");
    a->setEnabled(false);
    a->setVisible(false);

    MainWindow::instance()->setToolTip("Minitube");

    media->stop();
    media->clearQueue();
    currentVideoId.clear();

#ifdef APP_SNAPSHOT
    if (snapshotSettings) {
        delete snapshotSettings;
        snapshotSettings = nullptr;
    }
#endif
}

const QString &MediaView::getCurrentVideoId() {
    return currentVideoId;
}

void MediaView::activeVideoChanged(Video *video, Video *previousVideo) {
    if (stopped) return;

    media->stop();
    errorTimer->stop();

    if (previousVideo && previousVideo != video) {
        if (previousVideo->isLoadingStreamUrl()) previousVideo->abortLoadStreamUrl();
    }

    // optimize window for 16:9 video
    adjustWindowSize();

    videoAreaWidget->showLoading(video);

    connect(video, &Video::gotStreamUrl, this, &MediaView::gotStreamUrl, Qt::UniqueConnection);
    connect(video, SIGNAL(errorStreamUrl(QString)), SLOT(skip()), Qt::UniqueConnection);
    video->loadStreamUrl();

    // video title in titlebar
    MainWindow::instance()->setWindowTitle(video->getTitle() + QLatin1String(" - ") +
                                           QLatin1String(Constants::NAME));
    MainWindow::instance()->setToolTip(tr("Playing: %1").arg(video->getTitle()));

    // ensure active item is visible
    int row = playlistModel->rowForVideo(video);
    if (row != -1) {
        QModelIndex index = playlistModel->index(row, 0, QModelIndex());
        playlistView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }

    // enable/disable actions
    MainWindow::instance()
            ->getAction("download")
            ->setEnabled(DownloadManager::instance()->itemForVideo(video) == nullptr);
    MainWindow::instance()->getAction("previous")->setEnabled(row > 0);
    MainWindow::instance()->getAction("stopafterthis")->setEnabled(true);
    MainWindow::instance()->getAction("relatedVideos")->setEnabled(true);

    bool enableDownload = video->getLicense() == Video::LicenseCC;
#ifdef APP_ACTIVATION
    enableDownload = enableDownload || Activation::instance().isLegacy();
#endif
#ifdef APP_DOWNLOADS
    enableDownload = true;
#endif
    QAction *a = MainWindow::instance()->getAction("download");
    a->setEnabled(enableDownload);
    a->setVisible(enableDownload);

    updateSubscriptionActionForVideo(video, YTChannel::isSubscribed(video->getChannelId()));

    for (QAction *action : currentVideoActions)
        action->setEnabled(true);

#ifdef APP_SNAPSHOT
    if (snapshotSettings) {
        delete snapshotSettings;
        snapshotSettings = nullptr;
        MainWindow::instance()->adjustStatusBarVisibility();
    }
#endif

    // see you in gotStreamUrl...
}

void MediaView::gotStreamUrl(const QString &streamUrl, const QString &audioUrl) {
    if (stopped) return;
    if (streamUrl.isEmpty()) {
        qWarning() << "Empty stream url";
        skip();
        return;
    }

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender in" << __PRETTY_FUNCTION__;
        return;
    }
    video->disconnect(this);

    currentVideoId = video->getId();

    if (audioUrl.isEmpty()) {
        qDebug() << "Playing" << streamUrl;
        media->play(streamUrl);
    } else {
        qDebug() << "Playing" << streamUrl << audioUrl;
        media->playSeparateAudioAndVideo(streamUrl, audioUrl);
    }

    // ensure we always have videos ahead
    playlistModel->searchNeeded();

    // ensure active item is visible
    int row = playlistModel->activeRow();
    if (row != -1) {
        QModelIndex index = playlistModel->index(row, 0, QModelIndex());
        playlistView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }

#ifdef APP_ACTIVATION
    if (!demoTimer->isActive() && !Activation::instance().isActivated()) {
        int ms = (60000 * 2) + (QRandomGenerator::global()->generate() % (60000 * 2));
        demoTimer->start(ms);
    }
#endif

#ifdef APP_EXTRA
    Extra::notify(video->getTitle(), video->getChannelTitle(), video->getFormattedDuration());
#endif

    ChannelAggregator::instance()->videoWatched(video);
}

void MediaView::onItemActivated(const QModelIndex &index) {
    if (playlistModel->rowExists(index.row())) {
        // if it's the current video, just rewind and play
        Video *activeVideo = playlistModel->activeVideo();
        Video *video = playlistModel->videoAt(index.row());
        if (activeVideo && video && activeVideo == video) {
            media->play();
        } else
            playlistModel->setActiveRow(index.row());

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

void MediaView::onAboutToFinish() {

}

void MediaView::onPlaybackFinished() {
    if (stopped) return;

    const qint64 totalTime = media->duration();
    const qint64 currentTime = media->position();
    // qDebug() << __PRETTY_FUNCTION__ << mediaObject->currentTime() << totalTime;
    // add 10 secs for imprecise Phonon backends (VLC, Xine)
    if (currentTime > 0 && currentTime + 10000 < totalTime) {
        // mediaObject->seek(currentTime);
        QTimer::singleShot(500, this, SLOT(resumePlayback()));
    } else {
        QAction *stopAfterThisAction = MainWindow::instance()->getAction("stopafterthis");
        if (stopAfterThisAction->isChecked()) {
            stopAfterThisAction->setChecked(false);
        } else
            skip();
    }
}

void MediaView::resumePlayback() {
    if (stopped) return;
    const qint64 currentTime = media->position();
    // qDebug() << __PRETTY_FUNCTION__ << currentTime;
    if (currentTime > 0) media->seek(currentTime);
    media->play();
}

void MediaView::openWebPage() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    media->pause();
    QString url =
            video->getWebpage() + QLatin1String("&t=") + QString::number(media->position() / 1000);
    QDesktopServices::openUrl(url);
}

void MediaView::copyWebPage() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    QString address = video->getWebpage();
    QApplication::clipboard()->setText(address);
    QString message = tr("You can now paste the YouTube link into another application");
    MainWindow::instance()->showMessage(message);
}

void MediaView::copyVideoLink() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    QApplication::clipboard()->setText(video->getStreamUrl());
    QString message = tr("You can now paste the video stream URL into another application") + ". " +
                      tr("The link will be valid only for a limited time.");
    MainWindow::instance()->showMessage(message);
}

void MediaView::openInBrowser() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    media->pause();
    QDesktopServices::openUrl(video->getStreamUrl());
}

void MediaView::removeSelected() {
    if (!playlistView->selectionModel()->hasSelection()) return;
    QModelIndexList indexes = playlistView->selectionModel()->selectedIndexes();
    playlistModel->removeIndexes(indexes);
}

void MediaView::selectVideos(const QVector<Video *> &videos) {
    for (Video *video : videos) {
        QModelIndex index = playlistModel->indexForVideo(video);
        playlistView->selectionModel()->select(index, QItemSelectionModel::Select);
        playlistView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }
}

void MediaView::selectionChanged(const QItemSelection & /*selected*/,
                                 const QItemSelection & /*deselected*/) {
    const bool gotSelection = playlistView->selectionModel()->hasSelection();
    MainWindow::instance()->getAction("remove")->setEnabled(gotSelection);
    MainWindow::instance()->getAction("moveUp")->setEnabled(gotSelection);
    MainWindow::instance()->getAction("moveDown")->setEnabled(gotSelection);
}

void MediaView::moveUpSelected() {
    if (!playlistView->selectionModel()->hasSelection()) return;

    QModelIndexList indexes = playlistView->selectionModel()->selectedIndexes();
    std::stable_sort(indexes.begin(), indexes.end());
    playlistModel->move(indexes, true);

    // set current index after row moves to something more intuitive
    int row = indexes.at(0).row();
    playlistView->selectionModel()->setCurrentIndex(playlistModel->index(row > 1 ? row : 1),
                                                    QItemSelectionModel::NoUpdate);
}

void MediaView::moveDownSelected() {
    if (!playlistView->selectionModel()->hasSelection()) return;

    QModelIndexList indexes = playlistView->selectionModel()->selectedIndexes();
    std::stable_sort(indexes.begin(), indexes.end(),
                     [](const QModelIndex &a, const QModelIndex &b) { return b < a; });
    playlistModel->move(indexes, false);

    // set current index after row moves to something more intuitive
    // (respect 1 static item on bottom)
    int row = indexes.at(0).row() + 1, max = playlistModel->rowCount() - 2;
    playlistView->selectionModel()->setCurrentIndex(playlistModel->index(row > max ? max : row),
                                                    QItemSelectionModel::NoUpdate);
}

void MediaView::setSidebarVisibility(bool visible) {
    if (sidebar->isVisible() == visible) return;
    sidebar->setVisible(visible);
    if (visible) {
        sidebar->move(0, 0);
        sidebar->resize(sidebar->width(), window()->height());
        sidebar->raise();
        playlistView->setFocus();
    }
}

void MediaView::removeSidebar() {
    sidebar->hide();
    sidebar->setParent(window());
}

void MediaView::restoreSidebar() {
    sidebar->show();
    splitter->insertWidget(0, sidebar);
}

bool MediaView::isSidebarVisible() {
    return sidebar->isVisible();
}

void MediaView::saveSplitterState() {
    QSettings settings;
    if (splitter) settings.setValue("splitter", splitter->saveState());
}

void MediaView::downloadVideo() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    DownloadManager::instance()->addItem(video);
    MainWindow::instance()->showActionsInStatusBar({MainWindow::instance()->getAction("downloads")},
                                                   true);
    QString message = tr("Downloading %1").arg(video->getTitle());
    MainWindow::instance()->showMessage(message);
}

#ifdef APP_SNAPSHOT
void MediaView::snapshot() {
    qint64 currentTime = media->position() / 1000;

    QObject *context = new QObject();
    connect(media, &Media::snapshotReady, context,
            [this, currentTime, context](const QImage &image) {
                context->deleteLater();

                if (image.isNull()) {
                    qWarning() << "Null snapshot";
                    return;
                }

                QPixmap pixmap = QPixmap::fromImage(image.scaled(
                        videoWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                videoAreaWidget->showSnapshotPreview(pixmap);

                Video *video = playlistModel->activeVideo();
                if (!video) return;

                QString location = SnapshotSettings::getCurrentLocation();
                QDir dir(location);
                if (!dir.exists()) dir.mkpath(location);
                QString basename = video->getTitle();
                QString format = video->getDuration() > 3600 ? "h_mm_ss" : "m_ss";
                basename += " (" + QTime(0, 0, 0).addSecs(currentTime).toString(format) + ")";
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
    );

    media->snapshot();
}

void MediaView::fullscreen() {
    videoAreaWidget->setParent(nullptr);
    videoAreaWidget->showFullScreen();
}

void MediaView::resumeWithNewStreamUrl(const QString &streamUrl, const QString &audioUrl) {
    pauseTime = media->position();

    if (audioUrl.isEmpty()) {
        qDebug() << "Playing" << streamUrl;
        media->play(streamUrl);
    } else {
        qDebug() << "Playing" << streamUrl << audioUrl;
        media->playSeparateAudioAndVideo(streamUrl, audioUrl);
    }

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender in" << __PRETTY_FUNCTION__;
        return;
    }
    video->disconnect(this);
}

void MediaView::findVideoParts() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;

    QString query = video->getTitle();

    const QLatin1String optionalSpace("\\s*");
    const QLatin1String staticCounterSeparators("[\\/\\-]");
    const QString counterSeparators =
            QLatin1String("( of | ") + tr("of", "Used in video parts, as in '2 of 3'") +
            QLatin1String(" |") + staticCounterSeparators + QLatin1String(")");

    // numbers from 1 to 15
    const QLatin1String counterNumber("([1-9]|1[0-5])");

    // query.remove(QRegExp(counterSeparators + optionalSpace + counterNumber));
    query.remove(QRegularExpression(counterNumber + optionalSpace + counterSeparators +
                                    optionalSpace + counterNumber));
    query.remove(wordRE("pr?t\\.?" + optionalSpace + counterNumber));
    query.remove(wordRE("ep\\.?" + optionalSpace + counterNumber));
    query.remove(wordRE("part" + optionalSpace + counterNumber));
    query.remove(wordRE("episode" + optionalSpace + counterNumber));
    query.remove(wordRE(tr("part", "This is for video parts, as in 'Cool video - part 1'") +
                        optionalSpace + counterNumber));
    query.remove(wordRE(tr("episode", "This is for video parts, as in 'Cool series - episode 1'") +
                        optionalSpace + counterNumber));
    query.remove(QRegularExpression("[\\(\\)\\[\\]]"));

#define NUMBERS "one|two|three|four|five|six|seven|eight|nine|ten"

    QRegularExpression englishNumberRE = QRegularExpression(
            QLatin1String(".*(") + NUMBERS + ").*", QRegularExpression::CaseInsensitiveOption);
    // bool numberAsWords = englishNumberRE.exactMatch(query);
    query.remove(englishNumberRE);

    QRegularExpression localizedNumberRE = QRegularExpression(
            QLatin1String(".*(") + tr(NUMBERS) + ").*", QRegularExpression::CaseInsensitiveOption);
    // if (!numberAsWords) numberAsWords = localizedNumberRE.exactMatch(query);
    query.remove(localizedNumberRE);

    SearchParams *searchParams = new SearchParams();
    searchParams->setTransient(true);
    searchParams->setKeywords(query);
    searchParams->setChannelId(video->getChannelId());

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
    Video *video = playlistModel->activeVideo();
    if (!video) return;

    auto source = new SingleVideoSource(this);
    source->setVideo(video->clone());
    setVideoSource(source);

    MainWindow::instance()->getAction("relatedVideos")->setEnabled(false);
}

void MediaView::shareViaTwitter() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("https://twitter.com/intent/tweet");
    QUrlQuery q;
    q.addQueryItem("via", "minitubeapp");
    q.addQueryItem("text", video->getTitle());
    q.addQueryItem("url", video->getWebpage());
    url.setQuery(q);
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaFacebook() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("https://www.facebook.com/sharer.php");
    QUrlQuery q;
    q.addQueryItem("t", video->getTitle());
    q.addQueryItem("u", video->getWebpage());
    url.setQuery(q);
    QDesktopServices::openUrl(url);
}

void MediaView::shareViaEmail() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    QUrl url("mailto:");
    QUrlQuery q;
    q.addQueryItem("subject", video->getTitle());
    const QString body = video->getTitle() + "\n" + video->getWebpage() + "\n\n" +
                         tr("Sent from %1").arg(Constants::NAME) + "\n" + Constants::WEBSITE;
    q.addQueryItem("body", body);
    url.setQuery(q);
    QDesktopServices::openUrl(url);
}

void MediaView::onAuthorPushed(QModelIndex index) {
    Video *video = playlistModel->videoAt(index.row());
    if (!video) return;

    QString channelId = video->getChannelId();
    // if (channelId.isEmpty()) channelId = video->channelTitle();
    if (channelId.isEmpty()) return;

    SearchParams *searchParams = new SearchParams();
    searchParams->setChannelId(channelId);
    searchParams->setSortBy(SearchParams::SortByNewest);

    // go!
    search(searchParams);
}


void MediaView::updateSubscriptionAction(bool subscribed) {
    QAction *subscribeAction = MainWindow::instance()->getAction("subscribeChannel");

    QString subscribeTip;
    QString subscribeText;

    if (currentSubscriptionChannelId.isEmpty()) {
        subscribeText = subscribeAction->property("originalText").toString();
        subscribeAction->setEnabled(false);
    } else if (subscribed) {
        subscribeText = tr("Unsubscribe from %1").arg(currentSubscriptionChannelTitle);
        subscribeTip = subscribeText;
        subscribeAction->setEnabled(true);
    } else {
        subscribeText = tr("Subscribe to %1").arg(currentSubscriptionChannelTitle);
        subscribeTip = subscribeText;
        subscribeAction->setEnabled(true);
    }
    subscribeAction->setText(subscribeText);
    subscribeAction->setStatusTip(subscribeTip);

    if (subscribed) {
        subscribeAction->setIcon(IconUtils::icon("bookmark-remove"));
    } else {
        subscribeAction->setIcon(IconUtils::icon("bookmark-new"));
    }

    MainWindow::instance()->setupAction(subscribeAction);
}

void MediaView::updateSubscriptionActionForChannel(const QString & channelId) {
    QString channelTitle = tr("channel");
    YTChannel *channel = YTChannel::forId(channelId);
    if (nullptr != channel && !channel->getDisplayName().isEmpty()) {
        channelTitle = channel->getDisplayName();
    }

    bool subscribed = YTChannel::isSubscribed(channelId);

    currentSubscriptionChannelId = channelId;
    currentSubscriptionChannelTitle = channelTitle;
    updateSubscriptionAction(subscribed);
}

void MediaView::updateSubscriptionActionForVideo(Video *video, bool subscribed) {
    if (!video) {
        currentSubscriptionChannelId = "";
        currentSubscriptionChannelTitle = "";
        updateSubscriptionAction(false);
    } else {
        currentSubscriptionChannelId = video->getChannelId();
        currentSubscriptionChannelTitle = video->getChannelTitle();
        updateSubscriptionAction(subscribed);
    }
}

void MediaView::reloadCurrentVideo() {
    Video *video = playlistModel->activeVideo();
    if (!video) return;

    int oldFormat = video->getDefinitionCode();

    QObject *context = new QObject();
    connect(video, &Video::gotStreamUrl, context,
            [this, oldFormat, video, context](const QString &videoUrl, const QString &audioUrl) {
                context->deleteLater();
                if (oldFormat == video->getDefinitionCode()) return;
                QObject *context2 = new QObject();
                const qint64 position = media->position();
                connect(media, &Media::stateChanged, context2,
                        [position, this, context2](Media::State state) {
                            if (state == Media::PlayingState) {
                                media->seek(position);
                                context2->deleteLater();
                                Video *video = playlistModel->activeVideo();
                                QString msg = tr("Switched to %1")
                                                      .arg(VideoDefinition::forCode(
                                                                   video->getDefinitionCode())
                                                                   .getName());
                                MainWindow::instance()->showMessage(msg);
                            }
                        });

                if (audioUrl.isEmpty()) {
                    media->play(videoUrl);
                } else {
                    media->playSeparateAudioAndVideo(videoUrl, audioUrl);
                }
            });
    video->loadStreamUrl();
}

void MediaView::toggleSubscription() {
    //Video *video = playlistModel->activeVideo();
    if (currentSubscriptionChannelId.isEmpty()) {
        return;
    }

    bool subscribed = YTChannel::isSubscribed(currentSubscriptionChannelId);
    if (subscribed) {
        YTChannel::unsubscribe(currentSubscriptionChannelId);
        MainWindow::instance()->showMessage(
                tr("Unsubscribed from %1").arg(currentSubscriptionChannelTitle));
    } else {
        YTChannel::subscribe(currentSubscriptionChannelId);
        MainWindow::instance()->showMessage(tr("Subscribed to %1").arg(currentSubscriptionChannelTitle));
    }

    updateSubscriptionAction(!subscribed);
}

void MediaView::adjustWindowSize() {
    qDebug() << "Adjusting window size";
    Video *video = playlistModel->activeVideo();
    if (!video) return;
    QWidget *window = this->window();
    if (!window->isMaximized() && !window->isFullScreen()) {
        const double ratio = 16. / 9.;
        const double w = (double)videoAreaWidget->width();
        const double h = (double)videoAreaWidget->height();
        const double currentVideoRatio = w / h;
        if (currentVideoRatio != ratio) {
            qDebug() << "Adjust size";
            int newHeight = std::round((window->height() - h) + (w / ratio));
            window->resize(window->width(), newHeight);
        }
    }
}
