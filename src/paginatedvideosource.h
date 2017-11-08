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

#ifndef PAGINATEDVIDEOSOURCE_H
#define PAGINATEDVIDEOSOURCE_H

#include "videosource.h"

class PaginatedVideoSource : public VideoSource {

    Q_OBJECT

public:
    PaginatedVideoSource(QObject *parent = 0);
    virtual bool hasMoreVideos();

    bool maybeReloadToken(int max, int startIndex);
    bool setPageToken(const QString &value);
    bool isPageTokenExpired();
    void reloadToken();
    void setAsyncDetails(bool value) { asyncDetails = value; }
    void loadVideoDetails(const QVector<Video*> &videos);

signals:
    void gotDetails();

protected slots:
    void parseVideoDetails(const QByteArray &bytes);

protected:
    QString nextPageToken;
    uint tokenTimestamp;
    QUrl lastUrl;
    int currentMax;
    int currentStartIndex;
    bool reloadingToken;
    QVector<Video*> videos;
    QHash<QString, Video*> videoMap;
    bool asyncDetails;

};

#endif // PAGINATEDVIDEOSOURCE_H
