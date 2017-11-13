#include "cachedhttp.h"
#include "localcache.h"

namespace {

QByteArray requestHash(const HttpRequest &req) {
    const char sep = '|';
    QByteArray s = req.url.toEncoded() + sep + req.body + sep + QByteArray::number(req.offset);
    if (req.operation == QNetworkAccessManager::PostOperation) {
        s.append(sep);
        s.append("POST");
    }
    return LocalCache::hash(s);
}
}

CachedHttpReply::CachedHttpReply(const QByteArray &body, const HttpRequest &req)
    : bytes(body), req(req) {
    QTimer::singleShot(0, this, SLOT(emitSignals()));
}

QByteArray CachedHttpReply::body() const {
    return bytes;
}

void CachedHttpReply::emitSignals() {
    emit data(body());
    emit finished(*this);
    deleteLater();
}

WrappedHttpReply::WrappedHttpReply(LocalCache *cache, const QByteArray &key, QObject *httpReply)
    : QObject(httpReply), cache(cache), key(key), httpReply(httpReply) {
    connect(httpReply, SIGNAL(data(QByteArray)), SIGNAL(data(QByteArray)));
    connect(httpReply, SIGNAL(error(QString)), SIGNAL(error(QString)));
    connect(httpReply, SIGNAL(finished(HttpReply)), SLOT(originFinished(HttpReply)));
}

void WrappedHttpReply::originFinished(const HttpReply &reply) {
    if (reply.isSuccessful()) cache->insert(key, reply.body());
    emit finished(reply);
}

CachedHttp::CachedHttp(Http &http, const char *name)
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
    const QByteArray key = requestHash(req);
    const QByteArray value = cache->value(key);
    if (!value.isNull()) {
        qDebug() << "CachedHttp HIT" << req.url;
        return new CachedHttpReply(value, req);
    }
    qDebug() << "CachedHttp MISS" << req.url.toString();
    return new WrappedHttpReply(cache, key, http.request(req));
}
