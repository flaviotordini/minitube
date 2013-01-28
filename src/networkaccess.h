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
    QNetworkReply* getNetworkReply() { return networkReply; }

signals:
    void data(QByteArray);
    void error(QNetworkReply*);
    void finished(QNetworkReply*);

private slots:
    void finished();
    void requestError(QNetworkReply::NetworkError);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void readTimeout();

private:
    void setupReply();
    QNetworkReply *networkReply;
    QTimer *readTimeoutTimer;
    int retryCount;

};

class NetworkAccess : public QObject {

    Q_OBJECT

public:
    NetworkAccess(QObject* parent = 0);
    QNetworkReply* request(QUrl url,
                             int operation = QNetworkAccessManager::GetOperation,
                             const QByteArray &body = QByteArray());
    NetworkReply* get(QUrl url);
    NetworkReply* head(QUrl url);
    NetworkReply* post(QUrl url, const QMap<QString, QString>& params);

private:
    QNetworkRequest buildRequest(QUrl url);

};

#endif // NETWORKACCESS_H
