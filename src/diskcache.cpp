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

#include "diskcache.h"
#include <QtNetwork>

DiskCache::DiskCache(QObject *parent) : QNetworkDiskCache(parent) { }

QIODevice* DiskCache::prepare(const QNetworkCacheMetaData &metaData) {
    QString mime;
    const auto headers = metaData.rawHeaders();
    for (const QNetworkCacheMetaData::RawHeader &header : headers) {
        // qDebug() << header.first << header.second;
        if (header.first.constData() == QLatin1String("Content-Type")) {
            mime = header.second;
            break;
        }
    }

    if (mime == QLatin1String("application/json") || mime.startsWith(QLatin1String("image/"))) {
        return QNetworkDiskCache::prepare(metaData);
    }

    return 0;
}

QNetworkCacheMetaData DiskCache::metaData(const QUrl &url) {
    // Remove "key" from query string in order to reuse cache when key changes
    static const QString keyQueryItem = "key";
    QUrlQuery q(url);
    if (q.hasQueryItem(keyQueryItem)) {
        q.removeQueryItem(keyQueryItem);
        QUrl url2(url);
        url2.setQuery(q);
        return QNetworkDiskCache::metaData(url2);
    }

    return QNetworkDiskCache::metaData(url);
}
