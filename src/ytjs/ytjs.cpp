#include "ytjs.h"

#include "ytjsnamfactory.h"

#include "cachedhttp.h"
#include "http.h"
#include "httputils.h"

namespace {

QString wsBase = "https://flavio.tordini.org/minitube-ws/ytjs/";
QString ytJs = wsBase + "yt.js";

} // namespace

YTJS &YTJS::instance() {
    static YTJS i;
    return i;
}

Http &YTJS::http() {
    static Http *h = [] {
        Http *http = new Http;
        http->addRequestHeader("User-Agent", HttpUtils::userAgent());
        return http;
    }();
    return *h;
}

Http &YTJS::cachedHttp() {
    static Http *h = [] {
        CachedHttp *cachedHttp = new CachedHttp(http(), "ytjs");
        cachedHttp->setMaxSeconds(3600 * 6);
        cachedHttp->setIgnoreHostname(true);

        cachedHttp->getValidators().insert("application/javascript", [](const auto &reply) -> bool {
            return !reply.body().isEmpty();
        });

        return cachedHttp;
    }();
    return *h;
}

YTJS::YTJS(QObject *parent) : QObject(parent), engine(nullptr) {
    initialize();
}

bool YTJS::checkError(const QJSValue &value) {
    if (value.isError()) {
        qWarning() << "Error" << value.toString();
        qDebug() << value.property("stack").toString().splitRef('\n');
        return true;
    }
    return false;
}

bool YTJS::isInitialized() {
    if (ready) return true;
    initialize();
    return false;
}

void YTJS::initialize() {
    if (initializing) return;
    initializing = true;

    if (engine) engine->deleteLater();
    engine = new QQmlEngine(this);
    engine->setNetworkAccessManagerFactory(new YTJSNAMFactory);

    engine->globalObject().setProperty("global", engine->globalObject());

    QJSValue timer = engine->newQObject(new JsTimer(engine));
    engine->globalObject().setProperty("clearTimeout", timer.property("clearTimeout"));
    engine->globalObject().setProperty("setTimeout", timer.property("setTimeout"));

    connect(cachedHttp().get(ytJs), &HttpReply::finished, this, [this](auto &reply) {
        if (!reply.isSuccessful()) {
            emit initFailed("Cannot load " + ytJs);
            return;
        }
        evaluate(reply.body());
        ready = true;
        initializing = false;
        emit initialized();
    });
}

QJSValue YTJS::evaluate(const QString &js) {
    auto value = engine->evaluate(js);
    checkError(value);
    return value;
}
