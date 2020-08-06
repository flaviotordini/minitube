#include "ivchannel.h"

#include "http.h"
#include "httputils.h"
#include "invidious.h"

IVChannel::IVChannel(const QString &id, QObject *parent) : QObject(parent) {
    QUrl url = Invidious::instance().method("channels/");
    url.setPath(url.path() + id);

    auto *reply = Invidious::cachedHttp().get(url);
    connect(reply, &HttpReply::data, this, [this](auto data) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonObject obj = doc.object();

        displayName = obj["author"].toString();
        description = obj["descriptionHtml"].toString();

        const auto thumbnails = obj["authorThumbnails"].toArray();
        for (const auto &thumbnail : thumbnails) {
            if (thumbnail["width"].toInt() >= 300) {
                thumbnailUrl = thumbnail["url"].toString();
                break;
            }
        }
        qDebug() << displayName << description << thumbnailUrl;

        emit loaded();
    });
    connect(reply, &HttpReply::error, this, [this](auto message) {
        Invidious::instance().initServers();
        emit error(message);
    });
}
