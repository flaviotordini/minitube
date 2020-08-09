#ifndef INVIDIOUS_H
#define INVIDIOUS_H

#include <QtNetwork>

class Http;

class Invidious : public QObject {
    Q_OBJECT

public:
    static Invidious &instance();
    static Http &http();
    static Http &cachedHttp();

    explicit Invidious(QObject *parent = nullptr);
    void initServers();
    void shuffleServers();
    QString baseUrl();
    QUrl method(const QString &name);

signals:
    void serversInitialized();

private:
    QStringList servers;
};

#endif // INVIDIOUS_H
