#ifndef CACHEDHTTP_H
#define CACHEDHTTP_H

#include "http.h"

class LocalCache;

class CachedHttp : public Http {
public:
    CachedHttp(Http &http = Http::instance(), const QString &name = "http");
    void setMaxSeconds(uint seconds);
    void setMaxSize(uint maxSize);
    void setCachePostRequests(bool value) { cachePostRequests = value; }
    QObject *request(const HttpRequest &req);

private:
    Http &http;
    LocalCache *cache;
    bool cachePostRequests;
};

class CachedHttpReply : public HttpReply {
    Q_OBJECT

public:
    CachedHttpReply(LocalCache *cache, const QString &key, const HttpRequest &req);
    QUrl url() const { return req.url; }
    int statusCode() const { return 200; }
    QByteArray body() const;

private slots:
    void emitSignals();

private:
    LocalCache *cache;
    QString key;
    const HttpRequest &req;
};

class WrappedHttpReply : public QObject {
    Q_OBJECT

public:
    WrappedHttpReply(LocalCache *cache, const QString &key, QObject *httpReply);

signals:
    void data(const QByteArray &bytes);
    void error(const QString &message);
    void finished(const HttpReply &reply);

private slots:
    void originFinished(const HttpReply &reply);

private:
    LocalCache *cache;
    QString key;
    QObject *httpReply;
};

#endif // CACHEDHTTP_H
