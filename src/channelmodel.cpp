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

#include "channelmodel.h"
#include "channelaggregator.h"
#include "database.h"
#include "ytchannel.h"

namespace {
static const int channelOffset = 2;
}

ChannelModel::ChannelModel(QObject *parent)
    : QAbstractListModel(parent)
    , hoveredRow(-1)
    , sortBy(SortByName)
    , showUpdated(false) {
}

int ChannelModel::rowCount(const QModelIndex &) const {
    return channels.isEmpty() ? 0 : channelOffset + channels.size();
}

QVariant ChannelModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case ChannelModel::ItemTypeRole:
        return typeForIndex(index);

    case ChannelModel::DataObjectRole:
        if (typeForIndex(index) == ChannelModel::ItemChannel)
            return QVariant::fromValue(QPointer<YTChannel>(channelForIndex(index)));
        break;

    case ChannelModel::HoveredItemRole:
        return hoveredRow == index.row();

    case Qt::StatusTipRole:
        if (typeForIndex(index) == ChannelModel::ItemChannel)
            return channelForIndex(index)->getDescription();
    }

    return QVariant();
}

YTChannel* ChannelModel::channelForIndex(const QModelIndex &index) const {
    const int row = index.row();
    if (row < channelOffset) return 0;
    return channels.at(index.row() - channelOffset);
}

ChannelModel::ItemTypes ChannelModel::typeForIndex(const QModelIndex &index) const {
    switch (index.row()) {
    case 0:
        return ChannelModel::ItemAggregate;
    case 1:
        return ChannelModel::ItemUnwatched;
    default:
        return ChannelModel::ItemChannel;
    }
}

void ChannelModel::setQuery(const QString &query, const QSqlDatabase &db) {
    beginResetModel();
    channels.clear();
    sqlError = QSqlError();

    QSqlQuery q(db);
    q.prepare(query);
    bool success = q.exec();
    if (!success) {
        qWarning() << q.lastQuery() << q.lastError().text();
        sqlError = q.lastError();
    }
    while (q.next()) {
        YTChannel *channel = YTChannel::forId(q.value(0).toString());
        connect(channel, SIGNAL(thumbnailLoaded()), SLOT(updateSender()), Qt::UniqueConnection);
        connect(channel, SIGNAL(notifyCountChanged()), SLOT(updateSender()), Qt::UniqueConnection);
        connect(channel, SIGNAL(destroyed(QObject *)), SLOT(removeChannel(QObject *)), Qt::UniqueConnection);
        channels << channel;
    }
    endResetModel();
}

QSqlError ChannelModel::lastError() const {
    return sqlError;
}

void ChannelModel::updateSender() {
    YTChannel *channel = static_cast<YTChannel*>(sender());
    if (!channel) {
        qWarning() << "Cannot get sender" << __PRETTY_FUNCTION__;
        return;
    }
    updateChannel(channel);
}

void ChannelModel::updateChannel(YTChannel *channel) {
    int row = channels.indexOf(channel);
    if (row == -1) return;
    row += channelOffset;
    QModelIndex i = createIndex(row, 0);
    emit dataChanged(i, i);
}

void ChannelModel::updateUnwatched() {
    QModelIndex i = createIndex(1, 0);
    emit dataChanged(i, i);
}

void ChannelModel::removeChannel(QObject *obj) {
    YTChannel *channel = static_cast<YTChannel*>(obj);
    // qWarning() << "channel" << channel << obj << obj->metaObject()->className();
    if (!channel) return;

    int row = channels.indexOf(channel);
    if (row == -1) return;

    int position = row + channelOffset;
    beginRemoveRows(QModelIndex(), position, position+1);
    channels.removeAt(row);
    endRemoveRows();
}

void ChannelModel::setHoveredRow(int row) {
    int oldRow = hoveredRow;
    hoveredRow = row;
    emit dataChanged( createIndex( oldRow, 0 ), createIndex( oldRow, 0 ) );
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, 0 ) );
}

void ChannelModel::clearHover() {
    emit dataChanged( createIndex( hoveredRow, 0 ), createIndex( hoveredRow, 0 ) );
    hoveredRow = -1;
}

void ChannelModel::updateData() {
    if (!Database::exists()) return;

    QString sql = "select user_id from subscriptions";
    if (shouldShowUpdated())
        sql += " where notify_count>0";

    switch (getSortingOrder()) {
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

    setQuery(sql, Database::instance().getConnection());
    if (sqlError.isValid()) {
        qWarning() << sqlError.text();
    }
}

int ChannelModel::getUnwatchedCount() const {
    return ChannelAggregator::instance()->getUnwatchedCount();
}
