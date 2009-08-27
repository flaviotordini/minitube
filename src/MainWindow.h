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

// #include <QProgressDialog>

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

    /*
    struct DownloadResource
    {
        QProgressDialog* dialog;
        QFile* file;
    };*/

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
    void compactView(bool enable);
    void stop();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void searchFocus();
    void tick(qint64 time);
    void totalTimeChanged(qint64 time);

    // volume shortcuts
    void volumeUp();
    void volumeDown();
    void volumeMute();
    void volumeChanged(qreal newVolume);
    void volumeMutedChanged(bool muted);

    /*
    // download related stuff
    void abortDownload();
    void download();
    void download(const QUrl& url, const DownloadResource& res);
    void replyReadyRead();
    void replyDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void replyError(QNetworkReply::NetworkError code);
    void replyFinished();
    void replyMetaDataChanged();
    */

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
    QAction *compactViewAct;
    QAction *webPageAct;
    QAction *downloadAct;
    QAction *volumeUpAct;
    QAction *volumeDownAct;
    QAction *volumeMuteAct;

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
    bool m_maximized;

    // QMap<QNetworkReply*, DownloadResource> m_downloads;
};

#endif
