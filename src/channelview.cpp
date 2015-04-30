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

#include "channelview.h"
#include "ytchannel.h"
#include "ytsearch.h"
#include "searchparams.h"
#include "channelcontroller.h"
#include "channelmodel.h"
#include "channelitemdelegate.h"
#include "database.h"
#include "ytsearch.h"
#include "channelaggregator.h"
#include "aggregatevideosource.h"
#include "painterutils.h"
#include "mainwindow.h"
#include "iconutils.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif

ChannelView::ChannelView(QWidget *parent) : QListView(parent) {
    channelsModel = new ChannelModel(this);
    channelsController = new ChannelController(channelsModel, this);
    setItemDelegate(new ChannelItemDelegate(this));
    setSelectionMode(QAbstractItemView::NoSelection);

    // layout
    setSpacing(15);
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setUniformItemSizes(true);

    // cosmetics
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    QPalette p = palette();
    /*
    p.setColor(QPalette::Base, p.window().color());
    p.setColor(QPalette::Text, p.windowText().color());
    */
    p.setColor(QPalette::Disabled, QPalette::Base, p.base().color());
    p.setColor(QPalette::Disabled, QPalette::Text, p.text().color());
    setPalette(p);

    verticalScrollBar()->setPageStep(3);
    verticalScrollBar()->setSingleStep(1);

    setMouseTracking(true);

    connect(this, SIGNAL(clicked(const QModelIndex &)),
            SLOT(itemActivated(const QModelIndex &)));
    connect(this, SIGNAL(entered(const QModelIndex &)),
            SLOT(itemEntered(const QModelIndex &)));

    setModel(channelsModel);
    connect(this, SIGNAL(viewportEntered()),
            channelsModel, SLOT(clearHover()));

    setupActions();

    connect(ChannelAggregator::instance(), SIGNAL(channelChanged(YTChannel*)),
            channelsModel, SLOT(updateChannel(YTChannel*)));
    connect(ChannelAggregator::instance(), SIGNAL(unwatchedCountChanged(int)),
            SLOT(unwatchedCountChanged(int)));

    unwatchedCountChanged(ChannelAggregator::instance()->getUnwatchedCount());
}

void ChannelView::setupActions() {
    QMenu *sortMenu = new QMenu(this);
    QActionGroup *sortGroup = new QActionGroup(this);

    const SortBy sortBy = static_cast<SortBy>(channelsController->getSortingOrder());
    QAction *sortByNameAction = new QAction(tr("Name"), this);
    sortByNameAction->setActionGroup(sortGroup);
    sortByNameAction->setCheckable(true);
    if (sortBy == SortByName) sortByNameAction->setChecked(true);
    connect(sortByNameAction, SIGNAL(triggered()), SLOT(setSortByName()));
    sortMenu->addAction(sortByNameAction);

    QAction *sortByUpdatedAction = new QAction(tr("Last Updated"), this);
    sortByUpdatedAction->setActionGroup(sortGroup);
    sortByUpdatedAction->setCheckable(true);
    if (sortBy == SortByUpdated) sortByUpdatedAction->setChecked(true);
    connect(sortByUpdatedAction, SIGNAL(triggered()), SLOT(setSortByUpdated()));
    sortMenu->addAction(sortByUpdatedAction);

    QAction *sortByAddedAction = new QAction(tr("Last Added"), this);
    sortByAddedAction->setActionGroup(sortGroup);
    sortByAddedAction->setCheckable(true);
    if (sortBy == SortByAdded) sortByAddedAction->setChecked(true);
    connect(sortByAddedAction, SIGNAL(triggered()), SLOT(setSortByAdded()));
    sortMenu->addAction(sortByAddedAction);

    QAction *sortByLastWatched = new QAction(tr("Last Watched"), this);
    sortByLastWatched->setActionGroup(sortGroup);
    sortByLastWatched->setCheckable(true);
    if (sortBy == SortByLastWatched) sortByLastWatched->setChecked(true);
    connect(sortByLastWatched, SIGNAL(triggered()), SLOT(setSortByLastWatched()));
    sortMenu->addAction(sortByLastWatched);

    QAction *sortByMostWatched = new QAction(tr("Most Watched"), this);
    sortByMostWatched->setActionGroup(sortGroup);
    sortByMostWatched->setCheckable(true);
    if (sortBy == SortByMostWatched) sortByMostWatched->setChecked(true);
    connect(sortByMostWatched, SIGNAL(triggered()), SLOT(setSortByMostWatched()));
    sortMenu->addAction(sortByMostWatched);

    QToolButton *sortButton = new QToolButton(this);
    sortButton->setText(tr("Sort by"));
    sortButton->setIcon(IconUtils::icon("sort"));
    sortButton->setIconSize(QSize(16, 16));
    sortButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sortButton->setPopupMode(QToolButton::InstantPopup);
    sortButton->setMenu(sortMenu);
    QWidgetAction *widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(sortButton);
    widgetAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
    statusActions << widgetAction;

    markAsWatchedAction = new QAction(
                IconUtils::icon("mark-watched"), tr("Mark all as watched"), this);
    markAsWatchedAction->setEnabled(false);
    markAsWatchedAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_W));
    connect(markAsWatchedAction, SIGNAL(triggered()), SLOT(markAllAsWatched()));
    statusActions << markAsWatchedAction;

    const bool showUpdated = channelsController->shouldShowUpdated();
    QAction *showUpdatedAction = new QAction(
                IconUtils::icon("show-updated"), tr("Show Updated"), this);
    showUpdatedAction->setCheckable(true);
    showUpdatedAction->setChecked(showUpdated);
    showUpdatedAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_U));
    connect(showUpdatedAction, SIGNAL(toggled(bool)), SLOT(toggleShowUpdated(bool)));
    statusActions << showUpdatedAction;

    foreach (QAction *action, statusActions) {
        window()->addAction(action);
        IconUtils::setupAction(action);
    }
}

void ChannelView::appear() {
    channelsController->updateModelData();
    updateView();
    foreach (QAction* action, statusActions)
        MainWindow::instance()->showActionInStatusBar(action, true);
    setFocus();
    ChannelAggregator::instance()->start();
}

void ChannelView::disappear() {
    ChannelAggregator::instance()->stop();
    foreach (QAction* action, statusActions)
        MainWindow::instance()->showActionInStatusBar(action, false);
}

void ChannelView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton)
        showContextMenu(event->pos());
    else
        QListView::mousePressEvent(event);
}

void ChannelView::mouseMoveEvent(QMouseEvent *event) {
    QListView::mouseMoveEvent(event);
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) setCursor(Qt::PointingHandCursor);
    else unsetCursor();
}

void ChannelView::leaveEvent(QEvent *event) {
    QListView::leaveEvent(event);
    // channelsModel->clearHover();
}

void ChannelView::itemEntered(const QModelIndex &) {
    // channelsModel->setHoveredRow(index.row());
}

void ChannelView::itemActivated(const QModelIndex &index) {
    ChannelModel::ItemTypes itemType = channelsModel->typeForIndex(index);
    if (itemType == ChannelModel::ItemChannel) {
        YTChannel *channel = channelsModel->channelForIndex(index);
        SearchParams *params = new SearchParams();
        params->setChannelId(channel->getChannelId());
        params->setSortBy(SearchParams::SortByNewest);
        params->setTransient(true);
        YTSearch *videoSource = new YTSearch(params, this);
        videoSource->setAsyncDetails(true);
        emit activated(videoSource);
        channel->updateWatched();
    } else if (itemType == ChannelModel::ItemAggregate) {
        AggregateVideoSource *videoSource = new AggregateVideoSource(this);
        videoSource->setName(tr("All Videos"));
        emit activated(videoSource);
    } else if (itemType == ChannelModel::ItemUnwatched) {
        AggregateVideoSource *videoSource = new AggregateVideoSource(this);
        videoSource->setName(tr("Unwatched Videos"));
        videoSource->setUnwatched(true);
        emit activated(videoSource);
    }
}

void ChannelView::showContextMenu(const QPoint &point) {
    const QModelIndex index = indexAt(point);
    if (!index.isValid()) return;

    YTChannel *channel = channelsModel->channelForIndex(index);
    if (!channel) return;

    unsetCursor();

    QMenu menu;

    if (channel->getNotifyCount() > 0) {
        QAction *markAsWatchedAction = menu.addAction(tr("Mark as Watched"), channel, SLOT(updateWatched()));
        connect(markAsWatchedAction, SIGNAL(triggered()),
                ChannelAggregator::instance(), SLOT(updateUnwatchedCount()));
        menu.addSeparator();
    }

    /*
    // TODO
    QAction *notificationsAction = menu.addAction(tr("Receive Notifications"), user, SLOT(unsubscribe()));
    notificationsAction->setCheckable(true);
    notificationsAction->setChecked(true);
    */

    QAction *unsubscribeAction = menu.addAction(tr("Unsubscribe"), channel, SLOT(unsubscribe()));
    connect(unsubscribeAction, SIGNAL(triggered()),
            ChannelAggregator::instance(), SLOT(updateUnwatchedCount()));

    menu.exec(mapToGlobal(point));
}

void ChannelView::paintEvent(QPaintEvent *event) {
    if (model()->rowCount() < 3) {
        QString msg;
        if (channelsModel->lastError().isValid())
            msg = channelsModel->lastError().text();
        else if (channelsController->shouldShowUpdated())
            msg = tr("There are no updated subscriptions at this time.");
        else
            msg = tr("You have no subscriptions. "
                     "Use the star symbol to subscribe to channels.");
        PainterUtils::centeredMessage(msg, viewport());
    } else QListView::paintEvent(event);
    PainterUtils::topShadow(viewport());
}

void ChannelView::updateView(bool transition) {
#ifdef APP_EXTRA
    if (transition)
        Extra::fadeInWidget(this, this);
#else
    Q_UNUSED(transition);
#endif
}

void ChannelView::markAllAsWatched() {
    ChannelAggregator::instance()->markAllAsWatched();
    updateView();
    markAsWatchedAction->setEnabled(false);
    channelsController->markAllAsWatched();
}

void ChannelView::unwatchedCountChanged(int count) {
    markAsWatchedAction->setEnabled(count > 0);
    updateView();
    channelsController->unwatchedCountChanged(count);
}

void ChannelView::setSortBy(SortBy sortBy) {
    updateView(true);
    channelsController->setSortBy(static_cast<ChannelController::SortBy>(sortBy));
}

void ChannelView::toggleShowUpdated(bool enable) {
    updateView(true);
    channelsController->toggleShowUpdated(enable);
}
