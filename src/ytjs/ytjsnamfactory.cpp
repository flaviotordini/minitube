#include "ytjsnamfactory.h"

YTJSDiskCache::YTJSDiskCache(QObject *parent) : QNetworkDiskCache(parent) {}

void YTJSDiskCache::updateMetaData(const QNetworkCacheMetaData &meta) {
    auto meta2 = fixMetadata(meta);
    QNetworkDiskCache::updateMetaData(meta2);
}

QIODevice *YTJSDiskCache::prepare(const QNetworkCacheMetaData &meta) {
    auto meta2 = fixMetadata(meta);
    return QNetworkDiskCache::prepare(meta2);
}

QNetworkCacheMetaData YTJSDiskCache::fixMetadata(const QNetworkCacheMetaData &meta) {
    auto meta2 = meta;

    auto now = QDateTime::currentDateTimeUtc();
    if (meta2.expirationDate() < now) {
        meta2.setExpirationDate(now.addSecs(3600));
    }

    // Remove caching headers
    auto headers = meta2.rawHeaders();
    for (auto i = headers.begin(); i != headers.end(); ++i) {
        // qDebug() << i->first << i->second;
        if (i->first == "Cache-Control" || i->first == "Expires") {
            qDebug() << "Removing" << i->first << i->second;
            headers.erase(i);
        }
    }
    meta2.setRawHeaders(headers);

    return meta2;
}

YTJSNAM::YTJSNAM(QObject *parent) : QNetworkAccessManager(parent) {
    auto cache = new YTJSDiskCache(this);
    cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
                             "/ytjs");
    setCache(cache);
    setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

QNetworkReply *YTJSNAM::createRequest(QNetworkAccessManager::Operation op,
                                      const QNetworkRequest &request,
                                      QIODevice *outgoingData) {
    auto req2 = request;
    req2.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    qDebug() << req2.url();
    return QNetworkAccessManager::createRequest(op, req2, outgoingData);
}

QNetworkAccessManager *YTJSNAMFactory::create(QObject *parent) {
    qDebug() << "Creating NAM";
    return new YTJSNAM(parent);
}
