#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "faderwidget/FaderWidget.h"
#include "searchlineedit.h"
#include <phonon/audiooutput.h>
#include <phonon/volumeslider.h>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#include "View.h"
#include "SearchView.h"
#include "MediaView.h"
#include "AboutView.h"
#include "downloadview.h"

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();
    Phonon::SeekSlider* getSeekSlider() { return seekSlider; }

public slots:
    void showMedia(SearchParams *params);

protected:
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void fadeInWidget(QWidget *oldWidget, QWidget *newWidget);
    void goBack();
    void showSearch();
    void visitSite();
    void donate();
    void about();
    void quit();
    void fullscreen();
    void compactView(bool enable);
    void stop();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void searchFocus();
    void tick(qint64 time);
    void totalTimeChanged(qint64 time);
    void setDefinitionMode(QString definitionName);
    void toggleDefinitionMode();
    void clearRecentKeywords();

    // volume shortcuts
    void volumeUp();
    void volumeDown();
    void volumeMute();
    void volumeChanged(qreal newVolume);
    void volumeMutedChanged(bool muted);

    // fullscreen toolbar
    void showFullscreenToolbar(bool show);
    void showFullscreenPlaylist(bool show);

    // void setAutoplay(bool enabled);
    void updateDownloadMessage(QString);
    void downloadsFinished();
    void toggleDownloads(bool show);

    void startToolbarSearch(QString query);

private:
    void initPhonon();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void showWidget(QWidget*);
    static QString formatTime(qint64 time);

    // view mechanism
    QPointer<FaderWidget> faderWidget;
    QStackedWidget *views;
    QStack<QWidget*> *history;

    // view widgets
    SearchView *searchView;
    MediaView *mediaView;
    QWidget *aboutView;
    QWidget *downloadView;

    // actions
    QAction *addGadgetAct;
    QAction *backAct;
    QAction *quitAct;
    QAction *siteAct;
    QAction *donateAct;
    QAction *aboutAct;
    QAction *searchFocusAct;

    // media actions
    QAction *skipAct;
    QAction *pauseAct;
    QAction *stopAct;
    QAction *fullscreenAct;
    QAction *compactViewAct;
    QAction *webPageAct;
    QAction *copyPageAct;
    QAction *copyLinkAct;
    QAction *volumeUpAct;
    QAction *volumeDownAct;
    QAction *volumeMuteAct;

    // playlist actions
    QAction *removeAct;
    QAction *moveDownAct;
    QAction *moveUpAct;
    QAction *fetchMoreAct;
    QAction *clearAct;

    // menus
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *playlistMenu;
    QMenu *helpMenu;

    // toolbar
    QToolBar *mainToolBar;
    SearchLineEdit *toolbarSearch;

    // phonon
    Phonon::SeekSlider *seekSlider;
    // QSlider *slider;
    Phonon::VolumeSlider *volumeSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    QLabel *currentTime;
    QLabel *totalTime;

    bool m_fullscreen;
    bool m_maximized;

};

#endif
