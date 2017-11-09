#include "cachedhttp.h"
#include "localcache.h"

namespace {

QString requestHash(const HttpRequest &req) {
    const char sep = '|';
    QString s = req.url.toString() + sep + req.body + sep + QString::number(req.offset);
    if (req.operation == QNetworkAccessManager::PostOperation) {
        s += sep;
        s += QLatin1String("POST");
    }
    return LocalCache::hash(s);
}
}

CachedHttpReply::CachedHttpReply(LocalCache *cache, const QString &key, const HttpRequest &req)
    : cache(cache), key(key), req(req) {
    QTimer::singleShot(0, this, SLOT(emitSignals()));
}

QByteArray CachedHttpReply::body() const {
    return cache->value(key);
}

void CachedHttpReply::emitSignals() {
    emit data(body());
    emit finished(*this);
    deleteLater();
}

WrappedHttpReply::WrappedHttpReply(LocalCache *cache, const QString &key, QObject *httpReply)
    : QObject(httpReply), cache(cache), key(key), httpReply(httpReply) {
    connect(httpReply, SIGNAL(data(QByteArray)), SIGNAL(data(QByteArray)));
    connect(httpReply, SIGNAL(error(QString)), SIGNAL(error(QString)));
    connect(httpReply, SIGNAL(finished(HttpReply)), SLOT(originFinished(HttpReply)));
}

void WrappedHttpReply::originFinished(const HttpReply &reply) {
    if (reply.isSuccessful()) cache->insert(key, reply.body());
    emit finished(reply);
}

CachedHttp::CachedHttp(Http &http, const QString &name)
    : http(http), cache(LocalCache::instance(name)), cachePostRequests(false) {}

void CachedHttp::setMaxSeconds(uint seconds) {
    cache->setMaxSeconds(seconds);
}

void CachedHttp::setMaxSize(uint maxSize) {
    cache->setMaxSize(maxSize);
}

QObject *CachedHttp::request(const HttpRequest &req) {
    bool cacheable = req.operation == QNetworkAccessManager::GetOperation ||
                     (cachePostRequests && req.operation == QNetworkAccessManager::PostOperation);
    if (!cacheable) {
        qDebug() << "Not cacheable" << req.url;
        return http.request(req);
    }
    const QString key = requestHash(req);
    if (cache->isCached(key)) {
        // qDebug() << "CachedHttp HIT" << req.url;
        return new CachedHttpReply(cache, key, req);
    }
    // qDebug() << "CachedHttp MISS" << req.url.toString();
    return new WrappedHttpReply(cache, key, http.request(req));
}
