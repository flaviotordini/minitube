#ifndef NETWORKACCESS_H
#define NETWORKACCESS_H

#include <QtCore>
#include <QtNetwork>

namespace The {
    QNetworkAccessManager* networkAccessManager();
}

class NetworkReply : public QObject {

    Q_OBJECT

public:
    NetworkReply(QNetworkReply* networkReply);

public slots:
    void finished();
    void requestError(QNetworkReply::NetworkError);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void readTimeout();

signals:
    void data(QByteArray);
    void error(QNetworkReply*);
    void finished(QNetworkReply*);

private:
    QNetworkReply *networkReply;
    QTimer *readTimeoutTimer;

};

class NetworkAccess : public QObject {

    Q_OBJECT

public:
    NetworkAccess( QObject* parent=0);
    QNetworkReply* manualGet(QNetworkRequest request, int operation = QNetworkAccessManager::GetOperation);
    QNetworkRequest buildRequest(QUrl url);
    QNetworkReply* simpleGet(QUrl url, int operation = QNetworkAccessManager::GetOperation);
    NetworkReply* get(QUrl url);
    NetworkReply* head(QUrl url);

private slots:
    void error(QNetworkReply::NetworkError);

};

#endif // NETWORKACCESS_H
