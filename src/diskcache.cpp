#include "diskcache.h"
#include <QtNetwork>

DiskCache::DiskCache(QObject *parent) : QNetworkDiskCache(parent) { }

QIODevice* DiskCache::prepare(const QNetworkCacheMetaData &metaData) {
    QString mime;
    foreach (QNetworkCacheMetaData::RawHeader header, metaData.rawHeaders()) {
        // qDebug() << header.first << header.second;
        if (header.first.constData() == QLatin1String("Content-Type")) {
            mime = header.second;
            break;
        }
    }

    if (mime.startsWith(QLatin1String("image/")))
        return QNetworkDiskCache::prepare(metaData);

    return 0;
}
