#include "yt3.h"

#include <algorithm>
#include <ctime>

#include "jsfunctions.h"
#include "networkaccess.h"
#include "constants.h"
#include "compatibility/qurlqueryhelper.h"

#ifdef APP_EXTRA
#include "extra.h"
#endif

#define STR(x) #x
#define STRINGIFY(x) STR(x)

namespace The {
NetworkAccess* http();
}

YT3 &YT3::instance() {
    static YT3 *i = new YT3();
    return *i;
}

YT3::YT3() {
    QByteArray customApiKey = qgetenv("GOOGLE_API_KEY");
    if (!customApiKey.isEmpty()) {
        keys << QString::fromUtf8(customApiKey);
        qDebug() << "API key from environment" << keys;
    }

    if (keys.isEmpty()) {
        QSettings settings;
        if (settings.contains("googleApiKey")) {
            keys << settings.value("googleApiKey").toString();
            qDebug() << "API key from settings" << keys;
        }
    }

#ifdef APP_GOOGLE_API_KEY
    if (keys.isEmpty()) {
        keys << STRINGIFY(APP_GOOGLE_API_KEY);
        qDebug() << "built-in API key" << keys;
    }
#endif

#ifdef APP_EXTRA
    if (keys.isEmpty())
        keys << Extra::apiKeys();
#endif

    if (keys.isEmpty()) {
        qWarning() << "No available API keys";
    } else {
        key = keys.takeFirst();
        if (!keys.isEmpty()) testApiKey();
    }
}

const QString &YT3::baseUrl() {
    static const QString base = "https://www.googleapis.com/youtube/v3/";
    return base;
}

void YT3::testApiKey() {
    QUrl url = method("videos");
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("part", "id");
        urlHelper.addQueryItem("chart", "mostPopular");
        urlHelper.addQueryItem("maxResults", "1");
    }
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(testResponse(QNetworkReply*)));
}

void YT3::addApiKey(QUrl &url) {
    if (key.isEmpty()) {
        qDebug() << __PRETTY_FUNCTION__ << "empty key";
        return;
    }

    QUrlQueryHelper urlHelper(url);
    urlHelper.addQueryItem("key", key);
}

QUrl YT3::method(const QString &name) {
    QUrl url(baseUrl() + name);
    addApiKey(url);
    return url;
}

void YT3::testResponse(QNetworkReply *reply) {
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) {
        if (keys.isEmpty()) {
            qWarning() << "Fatal error: No working API keys!";
            return;
        }
        key = keys.takeFirst();
        testApiKey();
    } else {
        qDebug() << "Using key" << key;
    }
}
