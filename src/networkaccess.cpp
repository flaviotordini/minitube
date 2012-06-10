#include "networkaccess.h"
#include "constants.h"
#include <QtGui>

namespace The {
    NetworkAccess* http();
}

/*
const QString USER_AGENT = QString(Constants::NAME)
                           + " " + Constants::VERSION
                           + " (" + Constants::WEBSITE + ")";
*/

const QString USER_AGENT = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_3) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5";

NetworkReply::NetworkReply(QNetworkReply *networkReply) : QObject(networkReply) {
    this->networkReply = networkReply;

    // monitor downloadProgress to impl timeout
    connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)), Qt::UniqueConnection);

    readTimeoutTimer = new QTimer(this);
    readTimeoutTimer->setInterval(5000);
    readTimeoutTimer->setSingleShot(true);
    connect(readTimeoutTimer, SIGNAL(timeout()), SLOT(readTimeout()));
    readTimeoutTimer->start();
}

void NetworkReply::finished() {
    // qDebug() << "Finished" << networkReply->url();

    QUrl redirection = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirection.isValid()) {

        // qDebug() << "Redirect!"; // << redirection;

        QNetworkReply *redirectReply = The::http()->simpleGet(redirection, networkReply->operation());

        setParent(redirectReply);
        networkReply->deleteLater();
        networkReply = redirectReply;

        // when the request is finished we'll invoke the target method
        connect(networkReply, SIGNAL(finished()), this, SLOT(finished()), Qt::UniqueConnection);

        // monitor downloadProgress to impl timeout
        connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)),
                SLOT(downloadProgress(qint64,qint64)), Qt::UniqueConnection);
        readTimeoutTimer->start();

        // error signal
        connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
                SLOT(requestError(QNetworkReply::NetworkError)), Qt::UniqueConnection);

        connect(readTimeoutTimer, SIGNAL(timeout()), SLOT(readTimeout()), Qt::UniqueConnection);
        readTimeoutTimer->start();

        return;
    }

    emit data(networkReply->readAll());
    emit finished(networkReply);

#ifndef QT_NO_DEBUG_OUTPUT
    if (!networkReply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool()) {
        qDebug() << networkReply->url().toEncoded();
    }
#endif

    // bye bye my reply
    // this will also delete this NetworkReply as the QNetworkReply is its parent
    networkReply->deleteLater();
}

void NetworkReply::requestError(QNetworkReply::NetworkError /* code */) {
    emit error(networkReply);
}

void NetworkReply::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    // qDebug() << "Downloading" << bytesReceived << bytesTotal << networkReply->url();
    if (bytesReceived > 0) {
        readTimeoutTimer->stop();
        disconnect(networkReply, SIGNAL(downloadProgress(qint64,qint64)),
                   this, SLOT(downloadProgress(qint64,qint64)));
    }
}

void NetworkReply::readTimeout() {
    // qDebug() << "HTTP read timeout" << networkReply->url();
    networkReply->disconnect();
    networkReply->abort();

    QNetworkReply *retryReply = The::http()->simpleGet(networkReply->url(), networkReply->operation());

    setParent(retryReply);
    networkReply->deleteLater();
    networkReply = retryReply;

    // when the request is finished we'll invoke the target method
    connect(networkReply, SIGNAL(finished()), this, SLOT(finished()), Qt::UniqueConnection);

    // monitor downloadProgress to impl timeout
    connect(networkReply, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)), Qt::UniqueConnection);
    readTimeoutTimer->start();

    // error signal
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            SLOT(requestError(QNetworkReply::NetworkError)));

    // emit error(networkReply);
}

/* --- NetworkAccess --- */

NetworkAccess::NetworkAccess( QObject* parent) : QObject( parent ) {}

QNetworkReply* NetworkAccess::manualGet(QNetworkRequest request, int operation) {

    QNetworkAccessManager *manager = The::networkAccessManager();
    // manager->setCookieJar(new QNetworkCookieJar());

    QNetworkReply *networkReply;
    switch (operation) {

    case QNetworkAccessManager::GetOperation:
        // qDebug() << "GET" << request.url().toEncoded();
        networkReply = manager->get(request);
        break;

    case QNetworkAccessManager::HeadOperation:
        // qDebug() << "HEAD" << request.url().toEncoded();
        networkReply = manager->head(request);
        break;

    default:
        qDebug() << "Unknown operation:" << operation;
        return 0;

    }

    // error handling
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));

    return networkReply;
}

QNetworkRequest NetworkAccess::buildRequest(QUrl url) {
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT.toUtf8());
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    request.setRawHeader("Connection", "Keep-Alive");
    return request;
}

QNetworkReply* NetworkAccess::simpleGet(QUrl url, int operation) {
    return manualGet(buildRequest(url), operation);
}

NetworkReply* NetworkAccess::get(const QUrl url) {

    QNetworkReply *networkReply = simpleGet(url);
    NetworkReply *reply = new NetworkReply(networkReply);

    // error signal
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            reply, SLOT(requestError(QNetworkReply::NetworkError)));

    // when the request is finished we'll invoke the target method
    connect(networkReply, SIGNAL(finished()), reply, SLOT(finished()), Qt::UniqueConnection);

    return reply;

}

NetworkReply* NetworkAccess::head(const QUrl url) {

    QNetworkReply *networkReply = simpleGet(url, QNetworkAccessManager::HeadOperation);
    NetworkReply *reply = new NetworkReply(networkReply);

    // error signal
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            reply, SLOT(requestError(QNetworkReply::NetworkError)));

    // when the request is finished we'll invoke the target method
    connect(networkReply, SIGNAL(finished()), reply, SLOT(finished()), Qt::UniqueConnection);

    return reply;

}

void NetworkAccess::error(QNetworkReply::NetworkError code) {
    // get the QNetworkReply that sent the signal
    QNetworkReply *networkReply = static_cast<QNetworkReply *>(sender());
    if (!networkReply) {
        qDebug() << "Cannot get sender";
        return;
    }

    networkReply->deleteLater();

    // Ignore HEADs
    if (networkReply->operation() == QNetworkAccessManager::HeadOperation)
        return;

    // report the error in the status bar
    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(qApp->topLevelWidgets().first());
    if (mainWindow) mainWindow->statusBar()->showMessage(
            tr("Network error: %1").arg(networkReply->errorString()));

    qDebug() << "Network error:" << networkReply->errorString() << code;
}
