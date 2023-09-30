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

#include "homeview.h"
#include "channelaggregator.h"
#include "channelview.h"
#include "iconutils.h"
#include "mainwindow.h"
#include "mediaview.h"
#include "searchview.h"
#include "segmentedcontrol.h"
#include "standardfeedsview.h"
#ifdef APP_MAC
#include "macutils.h"
#endif

HomeView::HomeView(QWidget *parent)
    : QWidget(parent), searchView(nullptr), standardFeedsView(nullptr), channelsView(nullptr) {
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setupBar();
    layout->addWidget(bar);

    stackedWidget = new QStackedWidget();
    layout->addWidget(stackedWidget);
}

void HomeView::setupBar() {
    bar = new SegmentedControl();

    QAction *action = new QAction(tr("Search"), this);
    action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    action->setStatusTip(tr("Find videos and channels by keyword"));
    connect(action, SIGNAL(triggered()), SLOT(showSearch()));
    bar->addAction(action);
    bar->setCheckedAction(action);

    action = new QAction(tr("Browse"), this);
    action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    action->setStatusTip(tr("Browse videos by category"));
    connect(action, SIGNAL(triggered()), SLOT(showStandardFeeds()));
    bar->addAction(action);

    subscriptionsAction = new QAction(tr("Subscriptions"), this);
    subscriptionsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_3));
    subscriptionsAction->setStatusTip(tr("Channel subscriptions"));
    connect(subscriptionsAction, SIGNAL(triggered()), SLOT(showChannels()));
    bar->addAction(subscriptionsAction);
    connect(ChannelAggregator::instance(), SIGNAL(unwatchedCountChanged(int)),
            SLOT(unwatchedCountChanged(int)));

    const auto &a = bar->actions();
    for (QAction *action : a) {
        addAction(action);
        MainWindow::instance()->setupAction(action);
    }
}

void HomeView::showWidget(QWidget *widget) {
    QWidget *currentWidget = stackedWidget->currentWidget();
    if (currentWidget && currentWidget != widget) {
        currentWidget->setEnabled(false);
    }
    stackedWidget->setCurrentWidget(widget);
    widget->setEnabled(true);
}

void HomeView::showEvent(QShowEvent *event) {
    if (stackedWidget->count() == 0) showSearch();
}

void HomeView::showSearch() {
    if (!searchView) {
        searchView = new SearchView(this);
        connect(searchView, SIGNAL(search(SearchParams *)), MainWindow::instance(),
                SLOT(showMedia(SearchParams *)));
        stackedWidget->addWidget(searchView);
    }
    showWidget(searchView);
    bar->setCheckedAction(0);
}

void HomeView::showStandardFeeds() {
    if (!standardFeedsView) {
        standardFeedsView = new StandardFeedsView();
        connect(standardFeedsView, SIGNAL(activated(VideoSource *)), MainWindow::instance(),
                SLOT(showMedia(VideoSource *)));
        stackedWidget->addWidget(standardFeedsView);
    }
    showWidget(standardFeedsView);
    bar->setCheckedAction(1);
}

void HomeView::showChannels() {
    if (!channelsView) {
        channelsView = new ChannelView();
        connect(channelsView, SIGNAL(activated(VideoSource *)), MainWindow::instance(),
                SLOT(showMedia(VideoSource *)));
        stackedWidget->addWidget(channelsView);
    }
    showWidget(channelsView);
    bar->setCheckedAction(2);
}

void HomeView::unwatchedCountChanged(int count) {
    QVariant v;
    QString s;
    if (count > 0) {
        s = QString::number(count);
        v = s;
    }
    subscriptionsAction->setProperty("notifyCount", v);
    bar->update();
#ifdef APP_MAC
    mac::dockBadge(s);
#endif
}
