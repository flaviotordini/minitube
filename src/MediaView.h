#ifndef __MEDIAVIEW_H__
#define __MEDIAVIEW_H__

#include <QtGui>
#include <QtNetwork>
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>
#include "View.h"
#include "ListModel.h"
#include "thaction.h"
#include "thblackbar.h"
#include "searchparams.h"
#include "playlistwidget.h"
#include "loadingwidget.h"
#include "videoareawidget.h"

class MediaView : public QWidget, public View {
    Q_OBJECT

public:
    MediaView(QWidget *parent);
    ~MediaView();
    void initialize();

    // View
    void appear() {}
    void disappear();
    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        if (searchParams) {
            metadata.insert("title", searchParams->keywords());
            metadata.insert("description", tr("You're watching \"%1\"").arg(searchParams->keywords()));
        }
        return metadata;
    }

    void setMediaObject(Phonon::MediaObject *mediaObject);

public slots:
    void search(SearchParams *searchParams);
    void pause();
    void stop();
    void skip();
    void fullscreen();
    void exitFullscreen();
    void openWebPage();
    void removeSelected();
    void moveUpSelected();
    void moveDownSelected();
	 void setPlaylistVisible(bool visible=true);

private slots:
    // list/model
    void itemActivated(const QModelIndex &index);
    void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
    void activeRowChanged(int);
    void selectVideos(QList<Video*> videos);
    // phonon
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void aboutToFinish();
    void currentSourceChanged(const Phonon::MediaSource source);
    void showVideoContextMenu(QPoint point);
    void gotStreamUrl(QUrl streamUrl);
    // bar
    void searchMostRelevant();
    void searchMostRecent();
    void searchMostViewed();

private:

    SearchParams *searchParams;

    QSplitter *splitter;
    QByteArray splitterState;

    PlaylistWidget *playlistWidget;
    QListView *listView;
    ListModel *listModel;

    // sortBar
    THBlackBar *sortBar;
    THAction *mostRelevantAction;
    THAction *mostRecentAction;
    THAction *mostViewedAction;

    // phonon
    Phonon::MediaObject *mediaObject;
    Phonon::VideoWidget *videoWidget;

    // loadingWidget
    VideoAreaWidget *videoAreaWidget;
    LoadingWidget *loadingWidget;

};

#endif // __MEDIAVIEW_H__
