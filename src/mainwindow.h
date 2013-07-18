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
class VideoSource;

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
    MediaView* getMediaView() { return mediaView; }
    QToolButton* getRegionButton() { return regionButton; }
    QAction* getRegionAction() { return regionAction; }
    void showActionInStatusBar(QAction*, bool show);

public slots:
    void showHome(bool transition = true);
    void showMedia(SearchParams *params);
    void showMedia(VideoSource *videoSource);
    void showRegionsView();
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
    bool isReallyFullScreen();
    bool isCompact() { return m_compact; }

protected:
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void resizeEvent(QResizeEvent *);

private slots:
    void lazyInit();
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
    QList<QAction*> viewActions;

    // view widgets
    HomeView *homeView;
    MediaView *mediaView;
    QWidget *aboutView;
    QWidget *downloadView;
    QWidget *regionsView;

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

    // toolbar & statusbar
    QToolBar *mainToolBar;
    SearchLineEdit *toolbarSearch;
    QToolBar *statusToolBar;
    QToolButton *regionButton;
    QAction *regionAction;

    // phonon
    Phonon::SeekSlider *seekSlider;
    Phonon::VolumeSlider *volumeSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    QLabel *currentTime;
    QLabel *totalTime;

    // fullscreen
    bool m_fullscreen;
    bool m_maximized;
    QTimer *mouseTimer;
    bool m_compact;

};

#endif
