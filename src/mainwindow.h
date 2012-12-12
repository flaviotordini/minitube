#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <phonon/audiooutput.h>
#include <phonon/volumeslider.h>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#include "view.h"

class HomeView;
class MediaView;
class DownloadView;
class SearchLineEdit;
class UpdateChecker;
class SearchParams;

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    static MainWindow* instance();
    MainWindow();
    ~MainWindow();
    Phonon::SeekSlider* getSeekSlider() { return seekSlider; }
    void readSettings();
    void writeSettings();
    static void printHelp();

public slots:
    void showHome(bool transition = true);
    void showMedia(SearchParams *params);
    void restore();
    void messageReceived(const QString &message);
    void quit();
    void startToolbarSearch(QString query);
    void goBack();
    void showMessage(QString message);
#ifdef APP_ACTIVATION
    void showActivationView(bool transition = true);
    void showActivationDialog();
    void buy();
    void hideBuyAction();
#endif

protected:
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void resizeEvent(QResizeEvent *);

private slots:
    void checkForUpdate();
    void gotNewVersion(QString version);
    void visitSite();
    void donate();
    void reportIssue();
    void about();
    void fullscreen();
    void updateUIForFullscreen();
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

    void setManualPlay(bool enabled);
    void updateDownloadMessage(QString);
    void downloadsFinished();
    void toggleDownloads(bool show);

    void floatOnTop(bool);
    void showActionInStatusBar(QAction*, bool show);
    void showStopAfterThisInStatusBar(bool show);

    void hideMouse();

private:
    void initPhonon();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void showWidget(QWidget*, bool transition = true);
    static QString formatTime(qint64 time);
    bool confirmQuit();
    void simpleUpdateDialog(QString version);

    UpdateChecker *updateChecker;

    // view mechanism
    QStackedWidget *views;
    QStack<QWidget*> *history;

    // view widgets
    HomeView *homeView;
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
    QAction *skipBackwardAct;
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
    QAction *findVideoPartsAct;

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
    QToolBar *statusToolBar;

    // phonon
    Phonon::SeekSlider *seekSlider;
    Phonon::VolumeSlider *volumeSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    QLabel *currentTime;
    QLabel *totalTime;

    bool m_fullscreen;
    bool m_maximized;

    QTimer *mouseTimer;

};

#endif
