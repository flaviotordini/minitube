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

#ifndef CHANNELCONTROLLER_H
#define CHANNELCONTROLLER_H

#include <QtCore>

class ChannelModel;
class VideoSource;
class YTChannel;

class ChannelController : public QObject {

    Q_OBJECT

public:
    explicit ChannelController(QObject *parent = NULL);

    enum SortBy {
        SortByName = 0,
        SortByAdded,
        SortByUpdated,
        SortByLastWatched,
        SortByMostWatched
    };

    SortBy getSortingOrder() const { return sortBy; }
    bool shouldShowUpdated() const { return showUpdated; }
    ChannelModel *model() const { return channelModel; }

    void setSortBy(SortBy sortBy);
    void toggleShowUpdated(bool enable);
    void markAllAsWatched();
    void unwatchedCountChanged(int count);
    void updateModelData();
    void activateChannel(YTChannel *channel);
    void activateVideo(const QString &title, bool unwatched);

signals:
    void activated(VideoSource *videoSource);

private:
    ChannelModel *channelModel;
    SortBy sortBy;
    bool showUpdated;
};

#endif // CHANNELCONTROLLER_H
