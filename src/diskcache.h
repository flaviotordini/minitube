#ifndef DISKCACHE_H
#define DISKCACHE_H

#include <QNetworkDiskCache>

class DiskCache : public QNetworkDiskCache
{
    Q_OBJECT
public:
    explicit DiskCache(QObject *parent = 0);
    QIODevice* prepare(const QNetworkCacheMetaData &metaData);

signals:

public slots:

};

#endif // DISKCACHE_H
