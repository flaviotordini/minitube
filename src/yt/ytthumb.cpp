#include "ytthumb.h"

#include "http.h"
#include "httpreply.h"
#include "httputils.h"

YTThumb::YTThumb(int width, int height, const QString &url)
    : width(width), height(height), url(url) {}

VariantPromise &YTThumb::load(QObject *parent) {
    qDebug() << parent;
    if (promise) {
        qDebug() << "Already loading" << promise;
        return *promise;
    }
    promise = new VariantPromise(parent);
    promise->connect(HttpUtils::yt().get(url), &HttpReply::finished, promise, [this](auto &reply) {
        // clear promise member before emitting signals
        auto promise2 = promise;
        promise = nullptr;
        if (reply.isSuccessful()) {
            promise2->resolve(reply.body());
        } else {
            promise2->reject(reply.reasonPhrase());
        }
    });
    return *promise;
}
