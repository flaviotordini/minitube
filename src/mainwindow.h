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

#include "media.h"

class View;
class HomeView;
class MediaView;
class DownloadView;

class SearchLineEdit;
class SearchParams;
class VideoSource;
class Suggestion;
class ToolbarMenu;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static MainWindow *instance();
    MainWindow();

    QSlider *getSeekSlider() { return seekSlider; }
    QSlider *getVolumeSlider() { return volumeSlider; }

    QLabel *getCurrentTimeLabel() { return currentTimeLabel; }
    void readSettings();
    void writeSettings();
    static void printHelp();
    QStackedWidget *getViews() { return views; }
    MediaView *getMediaView() { return mediaView; }
    HomeView *getHomeView() { return homeView; }
    QAction *getRegionAction() { return regionAction; }
    SearchLineEdit *getToolbarSearch() { return toolbarSearch; }

    void setupAction(QAction *action);
    QAction *getAction(const char *name);
    void addNamedAction(const QByteArray &name, QAction *action);

    QMenu *getMenu(const char *name);

    void showActionsInStatusBar(const QVector<QAction *> &actions, bool show);
    void setStatusBarVisibility(bool show);
    void adjustStatusBarVisibility();

    void hideToolbar();
    void showToolbar();

#ifdef APP_ACTIVATION
    void showActivationView();
#endif

public slots:
    void showHome();
    void showMedia(SearchParams *params);
    void showMedia(VideoSource *videoSource);
    void showRegionsView();
    void restore();
    void messageReceived(const QString &message);
    void quit();
    void suggestionAccepted(Suggestion *suggestion);
    void search(const QString &query);
    bool canGoBack() { return history.size() > 1; }
    void goBack();
    void showMessage(const QString &message);
    void hideMessage();
    void handleError(const QString &message);
    bool isReallyFullScreen();
    bool isCompact() { return compactModeActive; }
    void missingKeyWarning();
    void visitSite();
    void setDefinitionMode(const QString &definitionName);

signals:
    void currentTimeChanged(const QString &s);
    void viewChanged();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    void showEvent(QShowEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void resizeEvent(QResizeEvent *e);
    void leaveEvent(QEvent *e);
    void enterEvent(QEvent *e);

private slots:
    void lazyInit();
    void donate();
    void reportIssue();
    void about();
    void toggleFullscreen();
    void updateUIForFullscreen();
    void compactView(bool enable);
    void stop();
    void searchFocus();
    void toggleDefinitionMode();
    void clearRecentKeywords();

    // media
    void stateChanged(Media::State state);
    void tick(qint64 time);

    void volumeUp();
    void volumeDown();
    void toggleVolumeMute();
    void volumeChanged(qreal newVolume);
    void volumeMutedChanged(bool muted);

    void updateDownloadMessage(const QString &);
    void downloadsFinished();
    void toggleDownloads(bool show);

    void setManualPlay(bool enabled);
    void floatOnTop(bool, bool showAction = true);
    void showStopAfterThisInStatusBar(bool show);
    void hideFullscreenUI();

    void toggleMenuVisibility();
    void toggleMenuVisibilityWithMessage();
    void toggleToolbarMenu();

#ifdef APP_MAC_STORE
    void rateOnAppStore();
#endif

private:
    void initMedia();
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void showView(View *view, bool transition = false);
    static QString formatTime(qint64 duration);
    bool confirmQuit();
    bool needStatusBar();
    void adjustMessageLabelPosition();

    QHash<QByteArray, QAction *> actionMap;
    QHash<QByteArray, QMenu *> menuMap;

    // view mechanism
    QStackedWidget *views;
    QStack<View *> history;

    // view widgets
    HomeView *homeView;
    MediaView *mediaView;
    View *aboutView;
    View *downloadView;
    View *regionsView;

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
    QSlider *seekSlider;
    QSlider *volumeSlider;
    QLabel *currentTimeLabel;

    bool fullScreenActive;
    bool maximizedBeforeFullScreen;
    bool menuVisibleBeforeFullScreen;
    QTimer *fullscreenTimer;
    bool compactModeActive;
    bool menuVisibleBeforeCompactMode;
    bool initialized;

    QLabel *messageLabel;
    QTimer *messageTimer;

    ToolbarMenu *toolbarMenu;
    QToolButton *toolbarMenuButton;

    Media *media;
};

#endif
