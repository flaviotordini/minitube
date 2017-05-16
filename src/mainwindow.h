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

#include <QtWidgets>
#ifdef APP_PHONON
#include <phonon/audiooutput.h>
#include <phonon/volumeslider.h>
#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#endif

class HomeView;
class MediaView;
class DownloadView;
class SearchLineEdit;
class UpdateChecker;
class SearchParams;
class VideoSource;
class Suggestion;

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    static MainWindow* instance();
    MainWindow();
#ifdef APP_PHONON_SEEK
    Phonon::SeekSlider* getSeekSlider() { return seekSlider; }
#else
    QSlider* getSlider() { return slider; }
#endif
#ifdef APP_PHONON
    Phonon::AudioOutput* getAudioOutput() { return audioOutput; }
    Phonon::VolumeSlider *getVolumeSlider() { return volumeSlider; }
#endif
    QLabel *getCurrentTimeLabel() { return currentTimeLabel; }
    void readSettings();
    void writeSettings();
    static void printHelp();
    MediaView* getMediaView() { return mediaView; }
    QAction* getRegionAction() { return regionAction; }
    SearchLineEdit *getToolbarSearch() { return toolbarSearch; }

    QHash<QString, QAction*> &getActionMap() { return actionMap; }
    QHash<QString, QMenu*> &getMenuMap() { return menuMap; }

    void showActionInStatusBar(QAction*, bool show);
    void setStatusBarVisibility(bool show);
    void adjustStatusBarVisibility();

    void hideToolbar();
    void showToolbar();

public slots:
    void showHome(bool transition = true);
    void showMedia(SearchParams *params);
    void showMedia(VideoSource *videoSource);
    void showRegionsView();
    void restore();
    void messageReceived(const QString &message);
    void quit();
    void suggestionAccepted(Suggestion *suggestion);
    void search(const QString &query);
    void goBack();
    void showMessage(const QString &message);
    void hideMessage();
#ifdef APP_ACTIVATION
    void showActivationView(bool transition = true);
    void showActivationDialog();
    void buy();
    void hideBuyAction();
#endif
    bool isReallyFullScreen();
    bool isCompact() { return compactModeActive; }
    void missingKeyWarning();

signals:
    void currentTimeChanged(const QString &s);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    void showEvent(QShowEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void resizeEvent(QResizeEvent *e);
    void moveEvent(QMoveEvent *e);
    void leaveEvent(QEvent *e);

private slots:
    void lazyInit();
    void checkForUpdate();
    void gotNewVersion(const QString &version);
    void visitSite();
    void donate();
    void reportIssue();
    void about();
    void fullscreen();
    void updateUIForFullscreen();
    void compactView(bool enable);
    void stop();
#ifdef APP_PHONON
    void stateChanged(Phonon::State newState, Phonon::State oldState);
#endif
    void searchFocus();
    void tick(qint64 time);
    void totalTimeChanged(qint64 time);
    void setDefinitionMode(const QString &definitionName);
    void toggleDefinitionMode();
    void clearRecentKeywords();

    void volumeUp();
    void volumeDown();
    void volumeMute();
    void volumeChanged(qreal newVolume);
    void volumeMutedChanged(bool muted);

    void updateDownloadMessage(const QString &);
    void downloadsFinished();
    void toggleDownloads(bool show);

    void setManualPlay(bool enabled);
    void floatOnTop(bool, bool showAction = true);
    void showStopAfterThisInStatusBar(bool show);
    void hideMouse();

    void toggleMenuVisibility();
    void toggleMenuVisibilityWithMessage();

#ifdef APP_MAC_STORE
    void rateOnAppStore();
#endif

private:
#ifdef APP_PHONON
    void initPhonon();
#endif
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void showWidget(QWidget*, bool transition = true);
    static QString formatTime(qint64 duration);
    bool confirmQuit();
    void simpleUpdateDialog(const QString &version);
    bool needStatusBar();
    void adjustMessageLabelPosition();

    UpdateChecker *updateChecker;

    QHash<QString, QAction*> actionMap;
    QHash<QString, QMenu*> menuMap;

    // view mechanism
    QStackedWidget *views;
    QStack<QWidget*> history;
    QList<QAction*> viewActions;

    // view widgets
    HomeView *homeView;
    MediaView *mediaView;
    QWidget *aboutView;
    QWidget *downloadView;
    QWidget *regionsView;

    // actions
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
    QAction *regionAction;

    // phonon
#ifdef APP_PHONON
#ifdef APP_PHONON_SEEK
    Phonon::SeekSlider *seekSlider;
#else
    QSlider *slider;
#endif
    Phonon::VolumeSlider *volumeSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
#endif
    QLabel *currentTimeLabel;

    bool fullScreenActive;
    bool maximizedBeforeFullScreen;
    bool menuVisibleBeforeFullScreen;
    QTimer *hideMouseTimer;
    bool compactModeActive;
    bool initialized;

    QLabel *messageLabel;
    QTimer *messageTimer;
};

#endif
