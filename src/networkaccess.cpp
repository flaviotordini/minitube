#include "networkaccess.h"
#include "constants.h"
#include <QtGui>

namespace The {
    NetworkAccess* http();
}

/*
const QString USER_AGENT = QString(Constants::APP_NAME)
                           + " " + Constants::VERSION
                           + " (" + Constants::WEBSITE + ")";
*/

const QString USER_AGENT = "Mozilla/5.0 (X11; U; Linux x86; en-US; rv:1.9.2.12) Gecko/20101028 Firefox/3.6.12";

NetworkReply::NetworkReply(QNetworkReply *networkReply) : QObject(networkReply) {
    this->networkReply = networkReply;
}

void NetworkReply::finished() {

    QUrl redirection = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirection.isValid()) {

        // qDebug() << "Redirect!"; // << redirection;

        QNetworkReply *redirectReply = The::http()->simpleGet(redirection, networkReply->operation());

        setParent(redirectReply);
        networkReply->deleteLater();
        networkReply = redirectReply;

        // when the request is finished we'll invoke the target method
        connect(networkReply, SIGNAL(finished()), this, SLOT(finished()), Qt::AutoConnection);

        return;
    }


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

QNetworkReply* NetworkAccess::manualGet(QNetworkRequest request, int operation) {

    QNetworkAccessManager *manager = The::networkAccessManager();

    QNetworkReply *networkReply;
    switch (operation) {

    case QNetworkAccessManager::GetOperation:
        qDebug() << "GET" << request.url().toEncoded();
        networkReply = manager->get(request);
        break;

    case QNetworkAccessManager::HeadOperation:
        qDebug() << "HEAD" << request.url().toEncoded();
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

QNetworkReply* NetworkAccess::simpleGet(QUrl url, int operation) {

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT.toUtf8());
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    // request.setRawHeader("Connection", "Keep-Alive");

    return manualGet(request, operation);
}

NetworkReply* NetworkAccess::get(const QUrl url) {

    QNetworkReply *networkReply = simpleGet(url);
    NetworkReply *reply = new NetworkReply(networkReply);

    // error signal
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            reply, SLOT(requestError(QNetworkReply::NetworkError)));

    // when the request is finished we'll invoke the target method
    connect(networkReply, SIGNAL(finished()), reply, SLOT(finished()), Qt::AutoConnection);

    return reply;

}

NetworkReply* NetworkAccess::head(const QUrl url) {

    QNetworkReply *networkReply = simpleGet(url, QNetworkAccessManager::HeadOperation);
    NetworkReply *reply = new NetworkReply(networkReply);

    // error signal
    connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            reply, SLOT(requestError(QNetworkReply::NetworkError)));

    // when the request is finished we'll invoke the target method
    connect(networkReply, SIGNAL(finished()), reply, SLOT(finished()), Qt::AutoConnection);

    return reply;

}

/*** sync ***/


QNetworkReply* NetworkAccess::syncGet(QUrl url) {

    working = true;

    networkReply = simpleGet(url);
    connect(networkReply, SIGNAL(metaDataChanged()),
            this, SLOT(syncMetaDataChanged()), Qt::AutoConnection);
    connect(networkReply, SIGNAL(finished()),
            this, SLOT(syncFinished()), Qt::AutoConnection);
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
                this, SLOT(metaDataChanged()), Qt::AutoConnection);
        connect(networkReply, SIGNAL(finished()),
                this, SLOT(finished()), Qt::AutoConnection);
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

    // Ignore HEADs
    if (networkReply->operation() == QNetworkAccessManager::HeadOperation)
        return;

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
