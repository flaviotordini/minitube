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

#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QtCore>
#include <QtSql>

class YTChannel;

class ChannelModel : public QAbstractListModel {

    Q_OBJECT

public:
    ChannelModel(QObject *parent = 0);

    enum DataRoles {
        ItemTypeRole = Qt::UserRole,
        DataObjectRole,
        HoveredItemRole
    };

    enum ItemTypes {
        ItemChannel = 1,
        ItemAggregate,
        ItemUnwatched
    };

    enum SortBy {
        SortByName = 0,
        SortByAdded,
        SortByUpdated,
        SortByLastWatched,
        SortByMostWatched
    };

    void setQuery(const QString &query, const QSqlDatabase &db);
    QSqlError lastError() const;
    ItemTypes typeForIndex(const QModelIndex &index) const;
    YTChannel* channelForIndex(const QModelIndex &index) const;
    void setHoveredRow(int row);
    void clearHover();

    SortBy getSortingOrder() const { return sortBy; }
    bool shouldShowUpdated() const { return showUpdated; }
    void setSortBy(SortBy sortingOrder) { sortBy = sortingOrder; }
    void toggleShowUpdated(bool enable) { showUpdated = enable; }
    int getUnwatchedCount() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    void updateData();

public slots:
    void updateSender();
    void updateChannel(YTChannel *channel);
    void updateUnwatched();
    void removeChannel(QObject *obj);

private:
    QList<YTChannel*> channels;
    int hoveredRow;
    QSqlError sqlError;
    SortBy sortBy;
    bool showUpdated;
};

#endif // CHANNELMODEL_H
