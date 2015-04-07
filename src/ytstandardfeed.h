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

#ifndef YTSTANDARDFEED_H
#define YTSTANDARDFEED_H

#include <QtNetwork>
#include "paginatedvideosource.h"

class YTStandardFeed : public PaginatedVideoSource {

    Q_OBJECT

public:
    YTStandardFeed(QObject *parent = 0);

    QString getFeedId() { return feedId; }
    void setFeedId(QString feedId) { this->feedId = feedId; }

    QString getRegionId() { return regionId; }
    void setRegionId(QString regionId) { this->regionId = regionId; }

    QString getCategory() { return category; }
    void setCategory(QString category) { this->category = category; }

    QString getLabel() { return label; }
    void setLabel(QString label) { this->label = label; }

    QString getTime() { return time; }
    void setTime(QString time) { this->time = time; }

    void loadVideos(int max, int startIndex);
    void abort();
    const QStringList & getSuggestions();
    QString getName() { return label; }

private slots:
    void parseResults(QByteArray data);
    void requestError(QNetworkReply *reply);

private:
    QString feedId;
    QString regionId;
    QString category;
    QString label;
    QString time;
    bool aborted;
};

#endif // YTSTANDARDFEED_H
