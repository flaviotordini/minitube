#ifndef __MEDIAVIEW_H__
#define __MEDIAVIEW_H__

#include <QtGui>
#include <QtNetwork>
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>
#include "View.h"
#include "ListModel.h"
#include "thlibrary/thblackbar.h"
#include "searchparams.h"
#include "playlistwidget.h"
#include "loadingwidget.h"
#include "videoareawidget.h"

class DownloadItem;

namespace The {
    QMap<QString, QAction*>* globalActions();
}

class MediaView : public QWidget, public View {
    Q_OBJECT

public:
    MediaView(QWidget *parent);
    void initialize();

    // View
    void appear() {
        listView->setFocus();
    }
    void disappear();
    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        if (searchParams) {
            metadata.insert("title", "");
            metadata.insert("description", "");
        }
        return metadata;
    }

    void setMediaObject(Phonon::MediaObject *mediaObject);
    void setSlider(QSlider *slider);

public slots:
    void search(SearchParams *searchParams);
    void pause();
    void stop();
    void skip();
    void skipVideo();
    void openWebPage();
    void copyWebPage();
    void copyVideoLink();
    void removeSelected();
    void moveUpSelected();
    void moveDownSelected();
    void setPlaylistVisible(bool visible=true);
    void saveSplitterState();
    void downloadVideo();
    void fullscreen();

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
    void showVideoContextMenu(QPoint point);
    // bar
    void searchMostRelevant();
    void searchMostRecent();
    void searchMostViewed();
    // timer
    void timerPlay();
#ifdef APP_DEMO
    void demoMessage();
#endif
    void startPlaying();
    void downloadStatusChanged();

    /*
    void downloadProgress(int percent);
    void sliderMoved(int value);
    void seekTo(int value);
    */

private:

    SearchParams *searchParams;

    QSplitter *splitter;

    PlaylistWidget *playlistWidget;
    QListView *listView;
    ListModel *listModel;

    // sortBar
    THBlackBar *sortBar;
    QAction *mostRelevantAction;
    QAction *mostRecentAction;
    QAction *mostViewedAction;

    // phonon
    Phonon::MediaObject *mediaObject;
    Phonon::VideoWidget *videoWidget;

    // loadingWidget
    VideoAreaWidget *videoAreaWidget;
    LoadingWidget *loadingWidget;

    bool timerPlayFlag;
    bool reallyStopped;

    QTimer *errorTimer;
    QTimer *workaroundTimer;
    Video *skippedVideo;

#ifdef APP_DEMO
    QTimer *demoTimer;
#endif

    DownloadItem *downloadItem;
    // QSlider *slider;

};

#endif // __MEDIAVIEW_H__
