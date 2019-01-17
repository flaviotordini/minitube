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

#ifndef MEDIAVIEW_H
#define MEDIAVIEW_H

#include <QtNetwork>
#include <QtWidgets>

#include "media.h"

#include "view.h"

class Video;
class PlaylistModel;
class SearchParams;
class LoadingWidget;
class VideoAreaWidget;
class PlaylistView;
class SidebarWidget;
class VideoSource;
#ifdef APP_SNAPSHOT
class SnapshotSettings;
#endif

class MediaView : public View {
    Q_OBJECT

public:
    static MediaView *instance();
    void initialize();

    void appear();
    void disappear();

    void setMedia(Media *media);
    const QVector<VideoSource *> &getHistory() { return history; }
    int getHistoryIndex();
    PlaylistModel *getPlaylistModel() { return playlistModel; }
    const QString &getCurrentVideoId();
    void updateSubscriptionAction(Video *video, bool subscribed);
    VideoAreaWidget *getVideoArea() { return videoAreaWidget; }
    void reloadCurrentVideo();

public slots:
    void search(SearchParams *searchParams);
    void setVideoSource(VideoSource *videoSource, bool addToHistory = true, bool back = false);
    void pause();
    void stop();
    void skip();
    void skipBackward();
    void skipVideo();
    void openWebPage();
    void copyWebPage();
    void copyVideoLink();
    void openInBrowser();
    void shareViaTwitter();
    void shareViaFacebook();
    void shareViaEmail();
    void removeSelected();
    void moveUpSelected();
    void moveDownSelected();
    bool isSidebarVisible();
    void setSidebarVisibility(bool visible);
    SidebarWidget *getSidebar() { return sidebar; }
    void removeSidebar();
    void restoreSidebar();
    void saveSplitterState();
    void downloadVideo();
#ifdef APP_SNAPSHOT
    void snapshot();
#endif
    void fullscreen();
    void findVideoParts();
    void relatedVideos();
    bool canGoBack();
    void goBack();
    bool canGoForward();
    void goForward();
    void toggleSubscription();
    void adjustWindowSize();

private slots:
    // list/model
    void itemActivated(const QModelIndex &index);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void activeVideoChanged(Video *video, Video *previousVideo);
    void selectVideos(const QVector<Video *> &videos);
    void gotStreamUrl(const QString &streamUrl, const QString &audioUrl);
    void handleError(const QString &message);
    void stateChanged(Media::State state);
    void aboutToFinish();
    void playbackFinished();
    void playbackResume();
    void authorPushed(QModelIndex);
    void searchAgain();
    void resumeWithNewStreamUrl(const QUrl &streamUrl);

private:
    MediaView(QWidget *parent = nullptr);
    SearchParams *getSearchParams();

    static QRegExp wordRE(const QString &s);

    QSplitter *splitter;
    SidebarWidget *sidebar;
    PlaylistView *playlistView;
    PlaylistModel *playlistModel;
    VideoAreaWidget *videoAreaWidget;
    LoadingWidget *loadingWidget;

    Media *media;
    QWidget *videoWidget;

    bool stopped;
    QTimer *errorTimer;
    Video *skippedVideo;
    QString currentVideoId;

#ifdef APP_ACTIVATION
    QTimer *demoTimer;
#endif

    QVector<VideoSource *> history;
    QVector<QAction *> currentVideoActions;

    qint64 currentVideoSize;

#ifdef APP_SNAPSHOT
    SnapshotSettings *snapshotSettings;
#endif

    QElapsedTimer pauseTimer;
    qint64 pauseTime;
};

#endif // MEDIAVIEW_H
