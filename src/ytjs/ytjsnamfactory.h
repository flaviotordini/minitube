#ifndef YTJSNAMFACTORY_H
#define YTJSNAMFACTORY_H

#include <QtQml>

class YTJSDiskCache : public QNetworkDiskCache {
public:
    YTJSDiskCache(QObject *parent);
    void updateMetaData(const QNetworkCacheMetaData &meta);
    QIODevice *prepare(const QNetworkCacheMetaData &meta);

private:
    QNetworkCacheMetaData fixMetadata(const QNetworkCacheMetaData &meta);
};

class YTJSNAM : public QNetworkAccessManager {
    Q_OBJECT

public:
    YTJSNAM(QObject *parent);

protected:
    QNetworkReply *
    createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);
};

class YTJSNAMFactory : public QQmlNetworkAccessManagerFactory {
public:
    QNetworkAccessManager *create(QObject *parent);
};

#endif // YTJSNAMFACTORY_H
