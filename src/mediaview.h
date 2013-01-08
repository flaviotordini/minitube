#ifndef __MEDIAVIEW_H__
#define __MEDIAVIEW_H__

#include <QtGui>
#include <QtNetwork>
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>
#include <phonon/seekslider.h>
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

namespace The {
    QMap<QString, QAction*>* globalActions();
}

class MediaView : public QWidget, public View {

    Q_OBJECT

public:
    static MediaView* instance();
    void initialize();

    void appear();
    void disappear();

    void setMediaObject(Phonon::MediaObject *mediaObject);
    const QList<VideoSource*> & getHistory() { return history; }
    int getHistoryIndex();
    PlaylistModel* getPlaylistModel() { return playlistModel; }

public slots:
    void search(SearchParams *searchParams);
    void setVideoSource(VideoSource *videoSource, bool addToHistory = true);
    void pause();
    void stop();
    void skip();
    void skipBackward();
    void skipVideo();
    void openWebPage();
    void copyWebPage();
    void copyVideoLink();
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
    void snapshot();
    void fullscreen();
    void findVideoParts();
    void relatedVideos();
    bool canGoBack();
    void goBack();
    bool canGoForward();
    void goForward();

private slots:
    // list/model
    void itemActivated(const QModelIndex &index);
    void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
    void activeRowChanged(int);
    void selectVideos(QList<Video*> videos);
    void gotStreamUrl(QUrl streamUrl);
    void handleError(QString message);
    // phonon
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void currentSourceChanged(const Phonon::MediaSource source);
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

    /*
    void downloadProgress(int percent);
    void sliderMoved(int value);
    void seekTo(int value);
    */

private:
    MediaView(QWidget *parent = 0);
    SearchParams* getSearchParams();
    static QRegExp wordRE(QString s);

    QSplitter *splitter;
    SidebarWidget *sidebar;
    PlaylistView *playlistView;
    PlaylistModel *playlistModel;
    VideoAreaWidget *videoAreaWidget;
    LoadingWidget *loadingWidget;

    // phonon
    Phonon::MediaObject *mediaObject;
    Phonon::VideoWidget *videoWidget;

    bool reallyStopped;
    QTimer *errorTimer;
    Video *skippedVideo;

#ifdef APP_ACTIVATION
    QTimer *demoTimer;
#endif

    DownloadItem *downloadItem;
    QList<VideoSource*> history;
};

#endif // __MEDIAVIEW_H__
