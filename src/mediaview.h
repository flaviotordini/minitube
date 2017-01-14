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

#ifndef __MEDIAVIEW_H__
#define __MEDIAVIEW_H__

#include <QtWidgets>
#include <QtNetwork>
#ifdef APP_PHONON
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>
#include <phonon/seekslider.h>
#endif
#include "view.h"

class Video;
class PlaylistModel;
class SearchParams;
class LoadingWidget;
class VideoAreaWidget;
class DownloadItem;
class PlaylistView;
class SidebarWidget;
class VideoSource;
#ifdef APP_SNAPSHOT
class SnapshotSettings;
#endif

namespace The {
QHash<QString, QAction*>* globalActions();
}

class MediaView : public View {

    Q_OBJECT

public:
    static MediaView* instance();
    void initialize();

    void appear();
    void disappear();

#ifdef APP_PHONON
    void setMediaObject(Phonon::MediaObject *mediaObject);
#endif
    const QList<VideoSource*> & getHistory() { return history; }
    int getHistoryIndex();
    PlaylistModel* getPlaylistModel() { return playlistModel; }
    const QString &getCurrentVideoId();
    void updateSubscriptionAction(Video *video, bool subscribed);
    VideoAreaWidget* getVideoArea() { return videoAreaWidget; }

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
    void shareViaBuffer();
    void shareViaEmail();
    void removeSelected();
    void moveUpSelected();
    void moveDownSelected();
    bool isPlaylistVisible();
    void setPlaylistVisible(bool visible=true);
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
    void maybeAdjustWindowSize();

private slots:
    // list/model
    void itemActivated(const QModelIndex &index);
    void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
    void activeRowChanged(int);
    void selectVideos(QList<Video*> videos);
    void gotStreamUrl(QUrl streamUrl);
    void handleError(const QString &message);
    // phonon
#ifdef APP_PHONON
    void stateChanged(Phonon::State newState, Phonon::State oldState);
#endif
    void aboutToFinish();
#ifdef APP_ACTIVATION
    void demoMessage();
    void updateContinueButton(int);
#endif
    void startPlaying();
    void downloadStatusChanged();
    void playbackFinished();
    void playbackResume();
    void authorPushed(QModelIndex);
    void searchAgain();
    void sliderMoved(int value);
    qint64 offsetToTime(qint64 offset);
    void startDownloading();
    void resumeWithNewStreamUrl(const QUrl &streamUrl);

private:
    MediaView(QWidget *parent = 0);
    SearchParams* getSearchParams();

    static QRegExp wordRE(const QString &s);

    QSplitter *splitter;
    SidebarWidget *sidebar;
    PlaylistView *playlistView;
    PlaylistModel *playlistModel;
    VideoAreaWidget *videoAreaWidget;
    LoadingWidget *loadingWidget;

#ifdef APP_PHONON
    Phonon::MediaObject *mediaObject;
    Phonon::VideoWidget *videoWidget;
#endif

    bool stopped;
    QTimer *errorTimer;
    Video *skippedVideo;
    QString currentVideoId;

#ifdef APP_ACTIVATION
    QTimer *demoTimer;
#endif

    DownloadItem *downloadItem;
    QList<VideoSource*> history;
    QList<QAction*> currentVideoActions;

    qint64 currentVideoSize;

#ifdef APP_SNAPSHOT
    SnapshotSettings *snapshotSettings;
#endif

    QElapsedTimer pauseTimer;
    qint64 pauseTime;
};

#endif // __MEDIAVIEW_H__
