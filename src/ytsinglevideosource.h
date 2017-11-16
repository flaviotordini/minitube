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

#ifndef YTSINGLEVIDEOSOURCE_H
#define YTSINGLEVIDEOSOURCE_H

#include <QtNetwork>
#include "paginatedvideosource.h"

class YTSingleVideoSource : public PaginatedVideoSource {

    Q_OBJECT

public:
    YTSingleVideoSource(QObject *parent = 0);
    void loadVideos(int max, int startIndex);
    void abort();
    QString getName();

    void setVideoId(const QString &value) { videoId = value; }
    void setVideo(Video *video);

private slots:
    void parseResults(QByteArray data);
    void requestError(const QString &message);

private:
    Video *video;
    QString videoId;
    bool aborted;
    int startIndex;
    int max;
    QString name;
};

#endif // YTSINGLEVIDEOSOURCE_H
