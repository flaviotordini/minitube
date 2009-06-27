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
#include "SettingsView.h"
#include "AboutView.h"

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void fadeInWidget(QWidget *oldWidget, QWidget *newWidget);
    void goBack();
    void showSettings();
    void showSearch();
    void showMedia(QString query);
    void visitSite();
    void donate();
    void about();
    void quit();
    void fullscreen();
    void stop();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void searchFocus();
    void tick(qint64 time);
    void totalTimeChanged(qint64 time);

private:
    void initPhonon();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void showWidget(QWidget*);

    // view mechanism
    QPointer<FaderWidget> faderWidget;
    QStackedWidget *views;
    QStack<QWidget*> *history;

    // view widgets
    QWidget *searchView;
    MediaView *mediaView;
    QWidget *settingsView;
    QWidget *aboutView;

    // actions
    QAction *addGadgetAct;
    QAction *settingsAct;
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
    QAction *webPageAct;
    QAction *downloadAct;

    // playlist actions
    QAction *removeAct;
    QAction *moveDownAct;
    QAction *moveUpAct;
    QAction *fetchMoreAct;

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
    Phonon::VolumeSlider *volumeSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    QLabel *currentTime;
    QLabel *totalTime;

    bool m_fullscreen;
};

#endif
