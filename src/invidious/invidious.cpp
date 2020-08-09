#include "invidious.h"

#include "cachedhttp.h"
#include "http.h"
#include "httputils.h"
#include "throttledhttp.h"

Invidious &Invidious::instance() {
    static Invidious i;
    return i;
}

Http &Invidious::http() {
    static Http *h = [] {
        Http *http = new Http;
        http->addRequestHeader("User-Agent", HttpUtils::stealthUserAgent());
        http->setMaxRetries(0);
        return http;
    }();
    return *h;
}

Http &Invidious::cachedHttp() {
    static Http *h = [] {
        ThrottledHttp *throttledHttp = new ThrottledHttp(http());
        throttledHttp->setMilliseconds(300);

        CachedHttp *cachedHttp = new CachedHttp(*throttledHttp, "iv");
        cachedHttp->setMaxSeconds(86400);
        cachedHttp->setIgnoreHostname(true);
        return cachedHttp;
    }();
    return *h;
}

Invidious::Invidious(QObject *parent) : QObject(parent) {}

void Invidious::initServers() {
    if (!servers.isEmpty()) shuffleServers();

    QUrl url("https://instances.invidio.us/instances.json?sort_by=type,health,users");
    auto reply = HttpUtils::yt().get(url);
    connect(reply, &HttpReply::finished, this, [this](auto &reply) {
        if (reply.isSuccessful()) {
            servers.clear();

            QSettings settings;
            QStringList keywords = settings.value("recentKeywords").toStringList();
            QString testKeyword = keywords.isEmpty() ? "test" : keywords.first();

            bool haveEnoughServers = false;
            QJsonDocument doc = QJsonDocument::fromJson(reply.body());
            for (const auto &v : doc.array()) {
                if (haveEnoughServers) break;

                auto serverArray = v.toArray();
                QString host = serverArray.first().toString();
                QJsonObject serverObj = serverArray.at(1).toObject();
                if (serverObj["type"] == "https") {
                    QString url = "https://" + host;

                    QUrl testUrl(url + "/api/v1/search?q=" + testKeyword);
                    auto reply = http().get(testUrl);
                    connect(reply, &HttpReply::finished, this,
                            [this, url, &haveEnoughServers](auto &reply) {
                                if (!haveEnoughServers && reply.isSuccessful()) {
                                    QJsonDocument doc = QJsonDocument::fromJson(reply.body());
                                    if (!doc.array().isEmpty()) {
                                        servers << url;
                                        if (servers.size() > 4) {
                                            haveEnoughServers = true;
                                            shuffleServers();
                                            qDebug() << servers;
                                            emit serversInitialized();
                                        }
                                    }
                                }
                            });
                }
            }
        }
    });
}

void Invidious::shuffleServers() {
    std::shuffle(servers.begin(), servers.end(), *QRandomGenerator::global());
}

QString Invidious::baseUrl() {
    QString host;
    if (servers.isEmpty())
        host = "https://invidious.snopyta.org";
    else
        host = servers.first();
    QString url = host + QLatin1String("/api/v1/");
    return url;
}

QUrl Invidious::method(const QString &name) {
    QUrl url(baseUrl() + name);
    return url;
}
