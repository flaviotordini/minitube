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
#include "ytuser.h"

static const int channelOffset = 2;

ChannelModel::ChannelModel(QObject *parent) :
    QAbstractListModel(parent),
    hoveredRow(-1) { }


int ChannelModel::rowCount(const QModelIndex &) const {
    return channels.isEmpty() ? 0 : channelOffset + channels.size();
}

QVariant ChannelModel::data(const QModelIndex &index, int role) const {
    switch (role) {

    case ChannelModel::ItemTypeRole:
        return typeForIndex(index);

    case ChannelModel::DataObjectRole:
        if (typeForIndex(index) == ChannelModel::ItemChannel)
            return QVariant::fromValue(QPointer<YTUser>(userForIndex(index)));
        break;

    case ChannelModel::HoveredItemRole:
        return hoveredRow == index.row();

    case Qt::StatusTipRole:
        if (typeForIndex(index) == ChannelModel::ItemChannel)
            return userForIndex(index)->getDescription();

    }

    return QVariant();
}

YTUser* ChannelModel::userForIndex(const QModelIndex &index) const {
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
        YTUser *user = YTUser::forId(q.value(0).toString());
        connect(user, SIGNAL(thumbnailLoaded()), SLOT(updateSender()), Qt::UniqueConnection);
        channels << user;
    }

    reset();
}

QSqlError ChannelModel::lastError() const {
    return sqlError;
}

void ChannelModel::updateSender() {
    YTUser *user = static_cast<YTUser*>(sender());
    if (!user) {
        qWarning() << "Cannot get sender" << __PRETTY_FUNCTION__;
        return;
    }
    updateChannel(user);
}

void ChannelModel::updateChannel(YTUser *user) {
    int row = channels.indexOf(user);
    if (row == -1) return;
    row += channelOffset;
    QModelIndex i = createIndex(row, 0);
    emit dataChanged(i, i);
}

void ChannelModel::updateUnwatched() {
    QModelIndex i = createIndex(1, 0);
    emit dataChanged(i, i);
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
