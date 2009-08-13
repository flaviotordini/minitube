#include "networkaccess.h"
#include "Constants.h"
#include <QtGui>

namespace The {
    NetworkAccess* http();
}

NetworkReply::NetworkReply(QNetworkReply *networkReply) : QObject(networkReply) {
    this->networkReply = networkReply;
}

void NetworkReply::metaDataChanged() {

    QUrl redirection = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirection.isValid()) {

        qDebug() << "Redirect" << redirection;

        QNetworkReply *redirectReply = The::http()->simpleGet(redirection);

        setParent(redirectReply);
        networkReply->deleteLater();
        networkReply = redirectReply;

        // handle redirections
        connect(networkReply, SIGNAL(metaDataChanged()),
                this, SLOT(metaDataChanged()), Qt::QueuedConnection);

        // when the request is finished we'll invoke the target method
        connect(networkReply, SIGNAL(finished()), this, SLOT(finished()), Qt::QueuedConnection);

    }
}

void NetworkReply::finished() {

    emit finished(networkReply);

    // get the HTTP response body
    QByteArray bytes = networkReply->readAll();

    emit data(bytes);

    // bye bye my reply
    // this will also delete this NetworkReply as the QNetworkReply is its parent
    networkReply->deleteLater();
}

void NetworkReply::requestError(QNetworkReply::NetworkError code) {
    emit error(networkReply);
}

/* --- NetworkAccess --- */

NetworkAccess::NetworkAccess( QObject* parent) : QObject( parent ) {}

QNetworkReply* NetworkAccess::simpleGet(QUrl url) {

    QNetworkAccessManager *manager = The::networkAccessManager();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", Constants::USER_AGENT.toUtf8());
    request.setRawHeader("Connection", "Keep-Alive");
    qDebug() << "GET" << url.toString();
    QNetworkReply *networkReply = manager->get(request);

    // error handling
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));

    return networkReply;

}

NetworkReply* NetworkAccess::get(const QUrl url) {

    QNetworkReply *networkReply = simpleGet(url);
    NetworkReply *reply = new NetworkReply(networkReply);

    // handle redirections
    connect(networkReply, SIGNAL(metaDataChanged()),
            reply, SLOT(metaDataChanged()), Qt::QueuedConnection);

    // error signal
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            reply, SLOT(requestError(QNetworkReply::NetworkError)));

    // when the request is finished we'll invoke the target method
    connect(networkReply, SIGNAL(finished()), reply, SLOT(finished()), Qt::QueuedConnection);

    return reply;

}

QNetworkReply* NetworkAccess::syncGet(QUrl url) {

    working = true;

    networkReply = simpleGet(url);
    connect(networkReply, SIGNAL(metaDataChanged()),
            this, SLOT(syncMetaDataChanged()), Qt::QueuedConnection);
    connect(networkReply, SIGNAL(finished()),
            this, SLOT(syncFinished()), Qt::QueuedConnection);
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));

    // A little trick to make this function blocking
    while (working) {
        // Do something else, maybe even network processing events
        qApp->processEvents();
    }

    networkReply->deleteLater();
    return networkReply;

}

void NetworkAccess::syncMetaDataChanged() {

    QUrl redirection = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirection.isValid()) {

        qDebug() << "Redirect" << redirection;
        networkReply->deleteLater();
        syncGet(redirection);

        /*
        QNetworkAccessManager *manager = The::networkAccessManager();
        networkReply->deleteLater();
        networkReply = manager->get(QNetworkRequest(redirection));
        connect(networkReply, SIGNAL(metaDataChanged()),
                this, SLOT(metaDataChanged()), Qt::QueuedConnection);
        connect(networkReply, SIGNAL(finished()),
                this, SLOT(finished()), Qt::QueuedConnection);
        */
    }

}

void NetworkAccess::syncFinished() {
    // got it!
    working = false;
}

void NetworkAccess::error(QNetworkReply::NetworkError code) {
    // get the QNetworkReply that sent the signal
    QNetworkReply *networkReply = static_cast<QNetworkReply *>(sender());
    if (!networkReply) {
        qDebug() << "Cannot get sender";
        return;
    }

    // report the error in the status bar
    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(qApp->topLevelWidgets().first());
    if (mainWindow) mainWindow->statusBar()->showMessage(
            tr("Network error: %1").arg(networkReply->errorString()));

    qDebug() << "Network error:" << networkReply->errorString() << code;

    networkReply->deleteLater();
}

QByteArray NetworkAccess::syncGetBytes(QUrl url) {
    return syncGet(url)->readAll();
}

QString NetworkAccess::syncGetString(QUrl url) {
    return QString::fromUtf8(syncGetBytes(url));
}
