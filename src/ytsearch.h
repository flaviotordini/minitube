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

#ifndef YTSEARCH_H
#define YTSEARCH_H

#include <QtNetwork>
#include "paginatedvideosource.h"

class SearchParams;
class Video;

class YTSearch : public PaginatedVideoSource {

    Q_OBJECT

public:
    YTSearch(SearchParams *params, QObject *parent = 0);
    void loadVideos(int max, int startIndex);
    void abort();
    const QStringList & getSuggestions();
    QString getName();
    QList<QAction*> getActions();
    SearchParams* getSearchParams() const { return searchParams; }
    static QString videoIdFromUrl(const QString &url);
    static QTime videoTimestampFromUrl(const QString &url);

    bool operator==(const YTSearch &other) const {
        return searchParams == other.getSearchParams();
    }

private slots:
    void parseResults(QByteArray data);
    void requestError(const QString &message);

private:
    SearchParams *searchParams;
    bool aborted;
    QStringList suggestions;
    QString name;
};

#endif // YTSEARCH_H
