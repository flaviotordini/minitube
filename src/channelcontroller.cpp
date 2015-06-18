/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2015, Flavio Tordini <flavio.tordini@gmail.com>

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

#include "aggregatevideosource.h"
#include "channelaggregator.h"
#include "channelcontroller.h"
#include "channelmodel.h"
#include "channelview.h"
#include "searchparams.h"
#include "ytchannel.h"
#include "ytsearch.h"

#include <QtDebug>

namespace {
static const char *sortByKey = "subscriptionsSortBy";
static const char *showUpdatedKey = "subscriptionsShowUpdated";
}  // namespace

ChannelController::ChannelController(QObject *parent)
    : QObject(parent),
    channelModel(NULL) {
    QSettings settings;
    const ChannelModel::SortBy sortOrder = static_cast<ChannelModel::SortBy>(settings.value(sortByKey, ChannelModel::SortByName).toInt());
    channelModel = new ChannelModel(this);
    channelModel->setSortBy(sortOrder);
    channelModel->toggleShowUpdated(settings.value(showUpdatedKey, false).toBool());
}

bool ChannelController::connectToView(ChannelView* channelsView) {
    bool connected = false;
    // Connect controller to the view
    connected = connect(channelsView, SIGNAL(onToggleShowUpdated(bool)),
                        SLOT(toggleShowUpdated(bool)));
    connected = connected && connect(channelsView, SIGNAL(onSortingOrderChanged(int)),
                                     SLOT(sortingOrderChanged(int)));
    connected = connected && connect(channelsView, SIGNAL(onUnwatchedCountChanged(int)),
                                     SLOT(unwatchedCountChanged(int)));
    connected = connected && connect(channelsView, SIGNAL(onMarkAllAsWatched()),
                                     SLOT(markAllAsWatched()));
    connected = connected && connect(channelsView, SIGNAL(onBeforeAppearance()),
                                     SLOT(onBeforeAppearance()));
    connected = connected && connect(channelsView, SIGNAL(onAppeared()),
                                     SLOT(onAppeared()));
    connected = connected && connect(channelsView, SIGNAL(onBeforeDisappearance()),
                                     SLOT(onBeforeDisappearance()));
    connected = connected && connect(channelsView, SIGNAL(onDisappeared()),
                                     SLOT(onDisappeared()));
    connected = connected && connect(channelsView, SIGNAL(onChannelActivated(YTChannel *)),
                                     SLOT(channelActivated(YTChannel *)));
    connected = connected && connect(channelsView, SIGNAL(onVideoActivated(const QString &, bool)),
                                     SLOT(videoActivated(const QString &, bool)));

    connected = connected && connect(ChannelAggregator::instance(), SIGNAL(channelChanged(YTChannel*)),
                                     channelModel, SLOT(updateChannel(YTChannel*)));
    connected = connected && connect(ChannelAggregator::instance(), SIGNAL(unwatchedCountChanged(int)),
                                     channelsView, SLOT(unwatchedCountChanged(int)));

    connected = connected && connect(channelsView, SLOT(onChannelUnsubscribe()),
                                     ChannelAggregator::instance(), SLOT(updateUnwatchedCount()));
    connected = connected && connect(channelsView, SLOT(onMarkChannelWatched()),
                                     ChannelAggregator::instance(), SLOT(updateUnwatchedCount()));

    connect(channelsView, SIGNAL(viewportEntered()),
            SLOT(clearHover()));
    return connected;
}

void ChannelController::updateModelData() {
    channelModel->updateData();
}

//
void ChannelController::toggleShowUpdated(bool enable) {
    channelModel->toggleShowUpdated(enable);
    updateModelData();
    QSettings settings;
    settings.setValue(showUpdatedKey, enable);
}

void ChannelController::sortingOrderChanged(int sortOrder) {
    channelModel->setSortBy(static_cast<ChannelModel::SortBy>(sortOrder));
    updateModelData();
    QSettings settings;
    settings.setValue(sortByKey, (int)sortOrder);
}

void ChannelController::unwatchedCountChanged(int) {
    //channelModel->updateUnwatched();
}

void ChannelController::markAllAsWatched() {
    ChannelAggregator::instance()->markAllAsWatched();
}

void ChannelController::onBeforeAppearance() {
    updateModelData();
}

void ChannelController::onAppeared() {
    ChannelAggregator::instance()->start();
}

void ChannelController::onBeforeDisappearance() {
    ChannelAggregator::instance()->stop();
}

void ChannelController::onDisappeared() {
}

void ChannelController::channelActivated(YTChannel *channel) {
    SearchParams *params = new SearchParams();
    params->setChannelId(channel->getChannelId());
    params->setSortBy(SearchParams::SortByNewest);
    params->setTransient(true);
    YTSearch *videoSource = new YTSearch(params, this);
    videoSource->setAsyncDetails(true);
    emit activated(videoSource);
    channel->updateWatched();
}

void ChannelController::videoActivated(const QString &title, bool unwatched) {
    AggregateVideoSource * const videoSource = new AggregateVideoSource(this);
    videoSource->setName(title);
    videoSource->setUnwatched(unwatched);
    emit activated(videoSource);
}

void ChannelController::clearHover() {
    channelModel->clearHover();
}
