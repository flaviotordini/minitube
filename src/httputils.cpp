#include "httputils.h"
#include "cachedhttp.h"
#include "constants.h"
#include "http.h"
#include "localcache.h"
#include "throttledhttp.h"

Http &HttpUtils::notCached() {
    static Http *h = [] {
        Http *http = new Http;
        http->addRequestHeader("User-Agent", userAgent());

        return http;
    }();
    return *h;
}

Http &HttpUtils::cached() {
    static Http *h = [] {
        Http *http = new Http;
        http->addRequestHeader("User-Agent", userAgent());

        CachedHttp *cachedHttp = new CachedHttp(*http, "http");

        return cachedHttp;
    }();
    return *h;
}

Http &HttpUtils::yt() {
    static Http *h = [] {
        Http *http = new Http;
        http->addRequestHeader("User-Agent", stealthUserAgent());

        CachedHttp *cachedHttp = new CachedHttp(*http, "yt");
        cachedHttp->setMaxSeconds(3600);

        return cachedHttp;
    }();
    return *h;
}

Http &HttpUtils::stealthAndNotCached() {
    static Http *h = [] {
        Http *http = new Http;
        http->addRequestHeader("User-Agent", stealthUserAgent());

        return http;
    }();
    return *h;
}

void HttpUtils::clearCaches() {
    LocalCache::instance("yt")->clear();
    LocalCache::instance("http")->clear();
}

const QByteArray &HttpUtils::userAgent() {
    static const QByteArray ua = [] {
        return QString(QLatin1String(Constants::NAME) + QLatin1Char('/') +
                       QLatin1String(Constants::VERSION) + QLatin1String(" ( ") +
                       Constants::WEBSITE + QLatin1String(" )"))
                .toUtf8();
    }();
    return ua;
}

const QByteArray &HttpUtils::stealthUserAgent() {
    static const QByteArray ua =
            "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/537.36 (KHTML, like "
            "Gecko) Chrome/79.0.3945.79 Safari/537.36";
    return ua;
}
