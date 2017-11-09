#include "throttledhttp.h"

namespace {

QElapsedTimer initElapsedTimer() {
    QElapsedTimer timer;
    timer.start();
    return timer;
}
}

ThrottledHttp::ThrottledHttp(Http &http) : http(http), elapsedTimer(initElapsedTimer()) {}

QObject *ThrottledHttp::request(const HttpRequest &req) {
    return new ThrottledHttpReply(http, req, milliseconds, elapsedTimer);
}

ThrottledHttpReply::ThrottledHttpReply(Http &http,
                                       const HttpRequest &req,
                                       int milliseconds,
                                       QElapsedTimer &elapsedTimer)
    : http(http), req(req), milliseconds(milliseconds), elapsedTimer(elapsedTimer), timer(0) {
    checkElapsed();
}

void ThrottledHttpReply::checkElapsed() {
    /*
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    */

    const qint64 elapsedSinceLastRequest = elapsedTimer.elapsed();
    if (elapsedSinceLastRequest < milliseconds) {
        if (!timer) {
            timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setTimerType(Qt::PreciseTimer);
            connect(timer, SIGNAL(timeout()), SLOT(checkElapsed()));
        }
        qDebug() << "Throttling" << req.url
                 << QString("%1ms").arg(milliseconds - elapsedSinceLastRequest);
        timer->setInterval(milliseconds - elapsedSinceLastRequest);
        timer->start();
        return;
    }
    elapsedTimer.start();
    doRequest();
}

void ThrottledHttpReply::doRequest() {
    QObject *reply = http.request(req);
    connect(reply, SIGNAL(data(QByteArray)), SIGNAL(data(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SIGNAL(error(QString)));
    connect(reply, SIGNAL(finished(HttpReply)), SIGNAL(finished(HttpReply)));

    // this will cause the deletion of this object once the request is finished
    setParent(reply);
}
