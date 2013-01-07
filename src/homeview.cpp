#include "homeview.h"
#include "segmentedcontrol.h"
#include "searchview.h"
#include "standardfeedsview.h"
#include "userview.h"
#include "mainwindow.h"
#include "mediaview.h"
#include "ytstandardfeed.h"

HomeView::HomeView(QWidget *parent) : QWidget(parent) {
    standardFeedsView = 0;
    userView = 0;

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    setupBar();
    layout->addWidget(bar);

    stackedWidget = new QStackedWidget();
    layout->addWidget(stackedWidget);

    searchView = new SearchView();
    connect(searchView, SIGNAL(search(SearchParams*)),
            MainWindow::instance(), SLOT(showMedia(SearchParams*)));
    stackedWidget->addWidget(searchView);
}

void HomeView::setupBar() {
    bar = new SegmentedControl(this);

    QAction *action = new QAction(tr("Search"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
    action->setStatusTip(tr("Find videos and channels by keyword"));
    connect(action, SIGNAL(triggered()), SLOT(showSearch()));
    bar->addAction(action);
    bar->setCheckedAction(action);

    action = new QAction(tr("Categories"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
    action->setStatusTip(tr("Browse videos by category"));
    connect(action, SIGNAL(triggered()), SLOT(showStandardFeeds()));
    bar->addAction(action);

    /*
    action = new QAction(tr("User"), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
    action->setStatusTip(tr("Your favorite videos, subscriptions and playlists"));
    connect(action, SIGNAL(triggered()), SLOT(showUser()));
    bar->addAction(action);
    */

    foreach (QAction* action, bar->actions()) {
        // action->setEnabled(false);
        addAction(action);
        action->setAutoRepeat(false);
        if (!action->shortcut().isEmpty())
            action->setStatusTip(
                        action->statusTip() + " (" +
                        action->shortcut().toString(QKeySequence::NativeText) + ")");
    }
}

void HomeView::showWidget(QWidget *widget) {
    QWidget* currentWidget = stackedWidget->currentWidget();
    if (currentWidget == widget) return;
    QMetaObject::invokeMethod(currentWidget, "disappear");
    currentWidget->setEnabled(false);
    stackedWidget->setCurrentWidget(widget);
    widget->setEnabled(true);
    QMetaObject::invokeMethod(widget, "appear");
    bar->setCheckedAction(stackedWidget->currentIndex());
    // autoChosenView = false;
    widget->setFocus();
}

void HomeView::appear() {
    QMetaObject::invokeMethod(stackedWidget->currentWidget(), "appear");
}

void HomeView::disappear() {
    QMetaObject::invokeMethod(stackedWidget->currentWidget(), "disappear");
}

void HomeView::showSearch() {
    showWidget(searchView);
}

void HomeView::showStandardFeeds() {
    if (!standardFeedsView) {
        standardFeedsView = new StandardFeedsView();
        connect(standardFeedsView, SIGNAL(activated(VideoSource*)),
                MainWindow::instance(),
                SLOT(showMedia(VideoSource*)));
        stackedWidget->addWidget(standardFeedsView);
    }
    showWidget(standardFeedsView);
}

void HomeView::showUser() {
    if (!userView) {
        userView = new UserView(this);
        stackedWidget->addWidget(userView);
    }
    showWidget(userView);
}
