#include "http.h"

namespace {

QNetworkAccessManager* createNetworkAccessManager() {
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    return nam;
}

QNetworkAccessManager *networkAccessManager() {
    static QHash<QThread*, QNetworkAccessManager*> nams;
    QThread *t = QThread::currentThread();
    QHash<QThread*, QNetworkAccessManager*>::const_iterator i = nams.constFind(t);
    if (i != nams.constEnd()) return i.value();
    QNetworkAccessManager* nam = createNetworkAccessManager();
    nams.insert(t, nam);
    return nam;
}

static int defaultReadTimeout = 10000;

}

Http::Http() :
    requestHeaders(getDefaultRequestHeaders()),
    readTimeout(defaultReadTimeout) {
}

void Http::setRequestHeaders(const QHash<QByteArray, QByteArray> &headers) {
    requestHeaders = headers;
}

QHash<QByteArray, QByteArray> &Http::getRequestHeaders() {
    return requestHeaders;
}

void Http::addRequestHeader(const QByteArray &name, const QByteArray &value) {
    requestHeaders.insert(name, value);
}

void Http::setReadTimeout(int timeout) {
    readTimeout = timeout;
}

Http &Http::instance() {
    static Http *i = new Http();
    return *i;
}

const QHash<QByteArray, QByteArray> &Http::getDefaultRequestHeaders() {
    static const QHash<QByteArray, QByteArray> defaultRequestHeaders = [] {
        QHash<QByteArray, QByteArray> h;
        h.insert("Accept-Charset", "utf-8");
        h.insert("Connection", "Keep-Alive");
        return h;
    }();
    return defaultRequestHeaders;
}

void Http::setDefaultReadTimeout(int timeout) {
    defaultReadTimeout = timeout;
}

QNetworkReply *Http::networkReply(const HttpRequest &req) {
    QNetworkRequest request(req.url);

    QHash<QByteArray, QByteArray> &headers = requestHeaders;
    if (!req.headers.isEmpty()) headers = req.headers;

    QHash<QByteArray, QByteArray>::const_iterator it;
    for (it = headers.constBegin(); it != headers.constEnd(); ++it)
        request.setRawHeader(it.key(), it.value());

    if (req.offset > 0)
        request.setRawHeader("Range", QString("bytes=%1-").arg(req.offset).toUtf8());

    QNetworkAccessManager *manager = networkAccessManager();

    QNetworkReply *networkReply = 0;
    switch (req.operation) {

    case QNetworkAccessManager::GetOperation:
        networkReply = manager->get(request);
        break;

    case QNetworkAccessManager::HeadOperation:
        networkReply = manager->head(request);
        break;

    case QNetworkAccessManager::PostOperation:
        networkReply = manager->post(request, req.body);
        break;

    default:
        qWarning() << "Unknown operation:" << req.operation;
    }

    return networkReply;
}

QObject* Http::request(const HttpRequest &req) {
    return new NetworkHttpReply(req, *this);
}

QObject* Http::request(const QUrl &url,
                       QNetworkAccessManager::Operation operation,
                       const QByteArray& body,
                       uint offset) {
    HttpRequest req;
    req.url = url;
    req.operation = operation;
    req.body = body;
    req.offset = offset;
    return request(req);
}

QObject* Http::get(const QUrl &url) {
    return request(url, QNetworkAccessManager::GetOperation);
}

QObject* Http::head(const QUrl &url) {
    return request(url, QNetworkAccessManager::HeadOperation);
}

QObject* Http::post(const QUrl &url, const QMap<QString, QString>& params) {
    QByteArray body;
    QMapIterator<QString, QString> i(params);
    while (i.hasNext()) {
        i.next();
        body += QUrl::toPercentEncoding(i.key())
                + '='
                + QUrl::toPercentEncoding(i.value())
                + '&';
    }
    HttpRequest req;
    req.url = url;
    req.operation = QNetworkAccessManager::PostOperation;
    req.body = body;
    req.headers = requestHeaders;
    req.headers.insert("Content-Type", "application/x-www-form-urlencoded");
    return request(req);
}

QObject* Http::post(const QUrl &url, QByteArray body, const QByteArray &contentType) {
    HttpRequest req;
    req.url = url;
    req.operation = QNetworkAccessManager::PostOperation;
    req.body = body;
    req.headers = requestHeaders;
    QByteArray cType = contentType;
    if (cType.isEmpty()) cType = "application/x-www-form-urlencoded";
    req.headers.insert("Content-Type", cType);
    return request(req);
}

NetworkHttpReply::NetworkHttpReply(const HttpRequest &req, Http &http) :
    http(http), req(req),
    retryCount(0) {

    if (req.url.isEmpty()) {
        qWarning() << "Empty URL";
    }

    networkReply = http.networkReply(req);
    setParent(networkReply);
    setupReply();

    readTimeoutTimer = new QTimer(this);
    readTimeoutTimer->setInterval(http.getReadTimeout());
    readTimeoutTimer->setSingleShot(true);
    connect(readTimeoutTimer, SIGNAL(timeout()), SLOT(readTimeout()), Qt::UniqueConnection);
    readTimeoutTimer->start();
}

void NetworkHttpReply::setupReply() {
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            SLOT(replyError(QNetworkReply::NetworkError)), Qt::UniqueConnection);
    connect(networkReply, SIGNAL(finished()),
            SLOT(replyFinished()), Qt::UniqueConnection);
    connect(networkReply, SIGNAL(downloadProgress(qint64, qint64)),
            SLOT(downloadProgress(qint64, qint64)), Qt::UniqueConnection);
}

QString NetworkHttpReply::errorMessage() {
    return url().toString() + QLatin1Char(' ') + QString::number(statusCode()) + QLatin1Char(' ') + reasonPhrase();
}

void NetworkHttpReply::emitError() {
    const QString msg = errorMessage();
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Http:" << msg;
    if (!req.body.isEmpty()) qDebug() << "Http:" << req.body;
#endif
    emit error(msg);
}

void NetworkHttpReply::replyFinished() {
    QUrl redirection = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirection.isValid()) {
        HttpRequest redirectReq;
        redirectReq.url = redirection;
        redirectReq.operation = req.operation;
        redirectReq.body = req.body;
        redirectReq.offset = req.offset;
        QNetworkReply *redirectReply = http.networkReply(redirectReq);
        setParent(redirectReply);
        networkReply->deleteLater();
        networkReply = redirectReply;
        setupReply();
        readTimeoutTimer->start();
        return;
    }

    if (isSuccessful()) {

        bytes = networkReply->readAll();
        emit data(bytes);

#ifndef QT_NO_DEBUG_OUTPUT
        if (!networkReply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool())
            qDebug() << networkReply->url().toString() << statusCode();
        else
            qDebug() << "CACHE" << networkReply->url().toString();
#endif
    }

    emit finished(*this);

    readTimeoutTimer->stop();

    // bye bye my reply
    // this will also delete this object and HttpReply as the QNetworkReply is their parent
    networkReply->deleteLater();
}

void NetworkHttpReply::replyError(QNetworkReply::NetworkError code) {
    Q_UNUSED(code);
    if (retryCount > 3) {
        emitError();
        return;
    }

    const int status = statusCode();
    if (status >= 500 && status < 600) {
        qDebug() << "Retrying" << req.url;
        networkReply->disconnect();
        networkReply->deleteLater();
        QNetworkReply *retryReply = http.networkReply(req);
        setParent(retryReply);
        networkReply = retryReply;
        setupReply();
        retryCount++;
        readTimeoutTimer->start();
    } else {
        emitError();
        return;
    }
}

void NetworkHttpReply::downloadProgress(qint64 bytesReceived, qint64 /* bytesTotal */) {
    // qDebug() << "Downloading" << bytesReceived << bytesTotal << networkReply->url();
    if (bytesReceived > 0 && readTimeoutTimer->isActive()) {
        readTimeoutTimer->stop();
        disconnect(networkReply, SIGNAL(downloadProgress(qint64,qint64)),
                   this, SLOT(downloadProgress(qint64,qint64)));
    }
}

void NetworkHttpReply::readTimeout() {
    if (!networkReply) return;
    networkReply->disconnect();
    networkReply->abort();
    networkReply->deleteLater();

    if (retryCount > 3 && (networkReply->operation() != QNetworkAccessManager::GetOperation
                           && networkReply->operation() != QNetworkAccessManager::HeadOperation)) {
        emitError();
        emit finished(*this);
        return;
    }

    qDebug() << "Timeout" << req.url;
    QNetworkReply *retryReply = http.networkReply(req);
    setParent(retryReply);
    networkReply = retryReply;
    setupReply();
    retryCount++;
    readTimeoutTimer->start();
}

QUrl NetworkHttpReply::url() const {
    return networkReply->url();
}

int NetworkHttpReply::statusCode() const {
    return networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

QString NetworkHttpReply::reasonPhrase() const {
    return networkReply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
}

const QList<QNetworkReply::RawHeaderPair> NetworkHttpReply::headers() const {
    return networkReply->rawHeaderPairs();
}

QByteArray NetworkHttpReply::header(const QByteArray &headerName) const {
    return networkReply->rawHeader(headerName);
}

QByteArray NetworkHttpReply:: body() const {
    return bytes;
}
