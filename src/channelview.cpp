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
#include "aggregatevideosource.h"
#include "channelaggregator.h"
#include "channelitemdelegate.h"
#include "channelmodel.h"
#include "database.h"
#include "iconutils.h"
#include "mainwindow.h"
#include "searchparams.h"
#include "ytchannel.h"
#include "ytsearch.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif
#include "channellistview.h"

#include "ivchannelsource.h"
#include "videoapi.h"
#include "ytjschannelsource.h"

namespace {
const QString sortByKey = "subscriptionsSortBy";
const QString showUpdatedKey = "subscriptionsShowUpdated";
} // namespace

ChannelView::ChannelView(QWidget *parent) : View(parent), showUpdated(false), sortBy(SortByName) {
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    listView = new ChannelListView();
    listView->setItemDelegate(new ChannelItemDelegate(this));

    channelsModel = new ChannelModel(this);
    listView->setModel(channelsModel);

    connect(listView, SIGNAL(clicked(const QModelIndex &)),
            SLOT(itemActivated(const QModelIndex &)));
    connect(listView, SIGNAL(contextMenu(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(listView, SIGNAL(viewportEntered()), channelsModel, SLOT(clearHover()));

    layout->addWidget(listView);

    setupActions();

    connect(ChannelAggregator::instance(), SIGNAL(channelChanged(YTChannel *)), channelsModel,
            SLOT(updateChannel(YTChannel *)));
    connect(ChannelAggregator::instance(), SIGNAL(unwatchedCountChanged(int)),
            SLOT(unwatchedCountChanged(int)));

    unwatchedCountChanged(ChannelAggregator::instance()->getUnwatchedCount());
}

void ChannelView::setupActions() {
    QSettings settings;

    statusActions << MainWindow::instance()->getAction("importSubscriptions");

    sortBy = static_cast<SortBy>(settings.value(sortByKey, SortByName).toInt());

    QMenu *sortMenu = new QMenu(this);
    QActionGroup *sortGroup = new QActionGroup(this);

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
    IconUtils::setIcon(sortButton, "sort");
    sortButton->setIconSize(QSize(16, 16));
    sortButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sortButton->setPopupMode(QToolButton::InstantPopup);
    sortButton->setMenu(sortMenu);
    QWidgetAction *widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(sortButton);
    widgetAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
    statusActions << widgetAction;

    markAsWatchedAction = new QAction(tr("Mark all as watched"), this);
    IconUtils::setIcon(markAsWatchedAction, "mark-watched");
    markAsWatchedAction->setEnabled(false);
    markAsWatchedAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_W));
    connect(markAsWatchedAction, SIGNAL(triggered()), SLOT(markAllAsWatched()));
    statusActions << markAsWatchedAction;

    showUpdated = settings.value(showUpdatedKey, false).toBool();
    QAction *showUpdatedAction = new QAction(tr("Show Updated"), this);
    IconUtils::setIcon(showUpdatedAction, "show-updated");
    showUpdatedAction->setCheckable(true);
    showUpdatedAction->setChecked(showUpdated);
    showUpdatedAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_U));
    connect(showUpdatedAction, SIGNAL(toggled(bool)), SLOT(toggleShowUpdated(bool)));
    statusActions << showUpdatedAction;

    for (QAction *action : statusActions) {
        window()->addAction(action);
        MainWindow::instance()->setupAction(action);
    }
}

QString ChannelView::noSubscriptionsMessage() {
    return tr("You have no subscriptions. "
              "Use the star symbol to subscribe to channels.");
}

void ChannelView::appear() {
    updateQuery(true);
    MainWindow::instance()->showActionsInStatusBar(statusActions, true);
    setFocus();
    ChannelAggregator::instance()->start();
}

void ChannelView::disappear() {
    MainWindow::instance()->showActionsInStatusBar(statusActions, false);
}

void ChannelView::itemActivated(const QModelIndex &index) {
    ChannelModel::ItemTypes itemType = channelsModel->typeForIndex(index);
    if (itemType == ChannelModel::ItemChannel) {
        YTChannel *channel = channelsModel->channelForIndex(index);
        SearchParams *params = new SearchParams();
        params->setChannelId(channel->getChannelId());
        params->setSortBy(SearchParams::SortByNewest);
        params->setTransient(true);
        VideoSource *vs = nullptr;
        if (VideoAPI::impl() == VideoAPI::YT3) {
            YTSearch *videoSource = new YTSearch(params);
            videoSource->setAsyncDetails(true);
            vs = videoSource;
        } else if (VideoAPI::impl() == VideoAPI::IV) {
            vs = new IVChannelSource(params);
        } else if (VideoAPI::impl() == VideoAPI::JS) {
            vs = new YTJSChannelSource(params);
        }
        emit activated(vs);
        channel->updateWatched();
    } else if (itemType == ChannelModel::ItemAggregate) {
        AggregateVideoSource *videoSource = new AggregateVideoSource();
        videoSource->setName(tr("All Videos"));
        emit activated(videoSource);
    } else if (itemType == ChannelModel::ItemUnwatched) {
        AggregateVideoSource *videoSource = new AggregateVideoSource();
        videoSource->setName(tr("Unwatched Videos"));
        videoSource->setUnwatched(true);
        emit activated(videoSource);
    }
}

void ChannelView::showContextMenu(const QPoint &point) {
    const QModelIndex index = listView->indexAt(point);
    if (!index.isValid()) return;

    YTChannel *channel = channelsModel->channelForIndex(index);
    if (!channel) return;

    unsetCursor();

    QMenu menu;

    if (channel->getNotifyCount() > 0) {
        QAction *markAsWatchedAction =
                menu.addAction(tr("Mark as Watched"), channel, SLOT(updateWatched()));
        connect(markAsWatchedAction, SIGNAL(triggered()), ChannelAggregator::instance(),
                SLOT(updateUnwatchedCount()));
        menu.addSeparator();
    }

    /*
    // TODO
    QAction *notificationsAction = menu.addAction(tr("Receive Notifications"), user,
    SLOT(unsubscribe())); notificationsAction->setCheckable(true);
    notificationsAction->setChecked(true);
    */

    QAction *unsubscribeAction = menu.addAction(tr("Unsubscribe"), channel, SLOT(unsubscribe()));
    connect(unsubscribeAction, SIGNAL(triggered()), ChannelAggregator::instance(),
            SLOT(updateUnwatchedCount()));

    menu.exec(mapToGlobal(point));
}

void ChannelView::toggleShowUpdated(bool enable) {
    showUpdated = enable;
    updateQuery(true);
    QSettings settings;
    settings.setValue(showUpdatedKey, showUpdated);
}

void ChannelView::updateQuery(bool transition) {
    Q_UNUSED(transition);
    listView->clearErrorMessage();

    if (!Database::exists()) {
        listView->setErrorMessage(noSubscriptionsMessage());
        return;
    }

    QString sql = "select user_id from subscriptions";
    if (showUpdated) sql += " where notify_count>0";

    switch (sortBy) {
    case SortByUpdated:
        sql += " order by updated desc";
        break;
    case SortByAdded:
        sql += " order by added desc";
        break;
    case SortByLastWatched:
        sql += " order by watched desc";
        break;
    case SortByMostWatched:
        sql += " order by views desc";
        break;
    default:
        sql += " order by name collate nocase";
        break;
    }

#ifdef APP_EXTRA
    if (transition) Extra::fadeInWidget(this, this);
#endif

    channelsModel->setQuery(sql, Database::instance().getConnection());
    if (channelsModel->lastError().isValid()) {
        qWarning() << channelsModel->lastError().text();
        listView->setErrorMessage(channelsModel->lastError().text());
    } else if (channelsModel->rowCount() < 3) {
        QString msg;
        if (showUpdated)
            msg = tr("There are no updated subscriptions at this time.");
        else
            msg = noSubscriptionsMessage();
        listView->setErrorMessage(msg);
    }
}

void ChannelView::setSortBy(SortBy sortBy) {
    this->sortBy = sortBy;
    updateQuery(true);
    QSettings settings;
    settings.setValue(sortByKey, (int)sortBy);
}

void ChannelView::markAllAsWatched() {
    ChannelAggregator::instance()->markAllAsWatched();
    updateQuery();
    markAsWatchedAction->setEnabled(false);
}

void ChannelView::unwatchedCountChanged(int count) {
    markAsWatchedAction->setEnabled(count > 0);
    channelsModel->updateUnwatched();
    updateQuery();
}
