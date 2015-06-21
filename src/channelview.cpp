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
#include "channelmodel.h"
#include "channelitemdelegate.h"
#include "ytchannel.h"
#include "painterutils.h"
#include "mainwindow.h"
#include "iconutils.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif

ChannelView::ChannelView(ChannelModel *model, QWidget *parent)
    : QListView(parent)
    , channelsModel(model) {
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
    unwatchedCountChanged(channelsModel->getUnwatchedCount());
}

void ChannelView::setupActions() {
    QMenu *sortMenu = new QMenu(this);
    QActionGroup *sortGroup = new QActionGroup(this);

    const SortBy sortBy = static_cast<SortBy>(channelsModel->getSortingOrder());
    QAction *sortByNameAction = createAction(
        tr("Name"), sortGroup, sortBy == SortByName, SLOT(setSortByName()));
    sortMenu->addAction(sortByNameAction);

    QAction *sortByUpdatedAction = createAction(
        tr("Last Updated"), sortGroup, sortBy == SortByUpdated, SLOT(setSortByUpdated()));
    sortMenu->addAction(sortByUpdatedAction);

    QAction *sortByAddedAction = createAction(
        tr("Last Added"), sortGroup, sortBy == SortByAdded, SLOT(setSortByAdded()));
    sortMenu->addAction(sortByAddedAction);

    QAction *sortByLastWatched = createAction(
        tr("Last Watched"), sortGroup, sortBy == SortByLastWatched, SLOT(setSortByLastWatched()));
    sortMenu->addAction(sortByLastWatched);

    QAction *sortByMostWatched = createAction(
        tr("Most Watched"), sortGroup, sortBy == SortByMostWatched, SLOT(setSortByMostWatched()));
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

    const bool showUpdated = channelsModel->shouldShowUpdated();
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

QAction *ChannelView::createAction(const QString &text, QActionGroup *sortGroup, bool isChecked, const char *slot) {
    QAction *action = new QAction(text, this);
    action->setActionGroup(sortGroup);
    action->setCheckable(true);
    action->setChecked(isChecked);
    connect(action, SIGNAL(triggered()), slot);
    return action;
}

void ChannelView::appear() {
    emit onBeforeAppearance();
    updateView();
    foreach (QAction* action, statusActions)
        MainWindow::instance()->showActionInStatusBar(action, true);
    setFocus();
    emit onAppeared();
}

void ChannelView::disappear() {
    emit onBeforeDisappearance();
    foreach (QAction* action, statusActions)
        MainWindow::instance()->showActionInStatusBar(action, false);
    emit onDisappeared();
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
}

void ChannelView::itemEntered(const QModelIndex &) {
}

void ChannelView::itemActivated(const QModelIndex &index) {
    ChannelModel::ItemTypes itemType = channelsModel->typeForIndex(index);
    if (itemType == ChannelModel::ItemChannel) {
        YTChannel *const channel = channelsModel->channelForIndex(index);
        emit onChannelActivated(channel);
    } else if (itemType == ChannelModel::ItemAggregate) {
        emit onVideoActivated(tr("All Videos"), false);
    } else if (itemType == ChannelModel::ItemUnwatched) {
        emit onVideoActivated(tr("Unwatched Videos"), true);
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
                this, SIGNAL(onMarkChannelWatched()));
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
            this, SIGNAL(onChannelUnsubscribe()));

    menu.exec(mapToGlobal(point));
}

void ChannelView::paintEvent(QPaintEvent *event) {
    if (model()->rowCount() < 3) {
        QString msg;
        if (channelsModel->lastError().isValid())
            msg = channelsModel->lastError().text();
        else if (channelsModel->shouldShowUpdated())
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
    updateView();
    markAsWatchedAction->setEnabled(false);
    emit onMarkAllAsWatched();
}

void ChannelView::unwatchedCountChanged(int count) {
    markAsWatchedAction->setEnabled(count > 0);
    updateView();
    emit onUnwatchedCountChanged(count);
}

void ChannelView::setSortBy(SortBy sortBy) {
    updateView(true);
    emit onSortingOrderChanged((int)sortBy);
}

void ChannelView::toggleShowUpdated(bool enable) {
    updateView(true);
    emit onToggleShowUpdated(enable);
}
