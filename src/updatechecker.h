#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QXmlStreamReader>
#include <QNetworkReply>

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    UpdateChecker();
    void checkForUpdate();
    QString remoteVersion();

signals:
    void newVersion(QString);

private slots:
    void requestFinished(QByteArray);

private:

    bool m_needUpdate;
    QString m_remoteVersion;
    QNetworkReply *networkReply;

};

class UpdateCheckerStreamReader : public QXmlStreamReader {

public:
    bool read(QByteArray data);
    QString remoteVersion();
    bool needUpdate() { return m_needUpdate; }

private:
    QString m_remoteVersion;
    bool m_needUpdate;

};

#endif // UPDATECHECKER_H
