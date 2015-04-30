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

#include "channelcontroller.h"
#include "channelaggregator.h"
#include "channelmodel.h"
#include "database.h"

#include <QtDebug>

namespace {
static const char *sortByKey = "subscriptionsSortBy";
static const char *showUpdatedKey = "subscriptionsShowUpdated";
}  // namespace

ChannelController::ChannelController(ChannelModel *model, QObject *parent)
    : QObject(parent),
    channelModel(model),
    sortBy(SortByName),
    showUpdated(false) {
    QSettings settings;
    sortBy = static_cast<SortBy>(settings.value(sortByKey, SortByName).toInt());
    showUpdated = settings.value(showUpdatedKey, false).toBool();
}

void ChannelController::setSortBy(SortBy sortBy) {
    this->sortBy = sortBy;
    updateModelData();
    QSettings settings;
    settings.setValue(sortByKey, (int)sortBy);
}

void ChannelController::toggleShowUpdated(bool enable) {
    showUpdated = enable;
    updateModelData();
    QSettings settings;
    settings.setValue(showUpdatedKey, showUpdated);
}

void ChannelController::updateModelData() {
    if (!Database::exists()) return;

    QString sql = "select user_id from subscriptions";
    if (showUpdated)
        sql += " where notify_count>0";

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

    channelModel->setQuery(sql, Database::instance().getConnection());
    if (channelModel->lastError().isValid()) {
        qWarning() << channelModel->lastError().text();
    }
}

void ChannelController::markAllAsWatched() {
    ChannelAggregator::instance()->markAllAsWatched();
}

void ChannelController::unwatchedCountChanged(int) {
    //ChannelModel->updateUnwatched();
}
