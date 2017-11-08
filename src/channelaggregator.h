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

#ifndef CHANNELAGGREGATOR_H
#define CHANNELAGGREGATOR_H

#include <QtCore>
#include <QtNetwork>

class YTChannel;
class Video;

class ChannelAggregator : public QObject {

    Q_OBJECT

public:
    static ChannelAggregator* instance();
    int getUnwatchedCount() { return unwatchedCount; }
    void markAllAsWatched();
    void videoWatched(Video *video);
    void cleanup();

public slots:
    void start();
    void stop();
    void run();
    void updateUnwatchedCount();

signals:
    void channelChanged(YTChannel*);
    void unwatchedCountChanged(int count);

private slots:
    void videosLoaded(const QVector<Video*> &videos);
    void processNextChannel();
    void checkWebPage(YTChannel *channel);
    void parseWebPage(const QByteArray &bytes);
    void errorWebPage(const QString &message);
    void reallyProcessChannel(YTChannel *channel);

private:
    ChannelAggregator(QObject *parent = 0);
    YTChannel* getChannelToCheck();
    void addVideo(Video* video);
    void finish();

    uint checkInterval;
    int unwatchedCount;
    bool running;

    int newVideoCount;
    QVector<YTChannel*> updatedChannels;

    QTimer *timer;
    bool stopped;

    YTChannel *currentChannel;
};

#endif // CHANNELAGGREGATOR_H
