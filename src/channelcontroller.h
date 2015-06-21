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
class ChannelView;
class VideoSource;
class YTChannel;

class ChannelController : public QObject {

    Q_OBJECT

public:
    explicit ChannelController(QObject *parent = NULL);

    ChannelModel *model() const { return channelModel; }
    bool connectToView(ChannelView* channelsView);

public slots:
    void toggleShowUpdated(bool enable);
    void sortingOrderChanged(int sortOrder);
    void unwatchedCountChanged(int count);
    void markAllAsWatched();
    void onBeforeAppearance();
    void onAppeared();
    void onBeforeDisappearance();
    void onDisappeared();
    void channelActivated(YTChannel *channel);
    void videoActivated(const QString &title, bool unwatched);
    void clearHover();

signals:
    void activated(VideoSource *videoSource);

private:
    void updateModelData();

    ChannelModel *channelModel;
};

#endif // CHANNELCONTROLLER_H
