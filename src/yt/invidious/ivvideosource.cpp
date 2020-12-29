#include "ivvideosource.h"

void IVVideoSource::loadVideos(int max, int startIndex) {
    aborted = false;
    retryCount = 0;
    this->max = max;
    this->startIndex = startIndex;
    reallyLoadVideos(max, startIndex);
}

void IVVideoSource::abort() {
    aborted = true;
    retryCount = 0;
    max = 0;
    startIndex = 0;
}

void IVVideoSource::handleError(QString message) {
    qDebug() << message;
    if (retryCount < 4) {
        qDebug() << "Retrying...";
        Invidious::instance().shuffleServers();
        reallyLoadVideos(max, startIndex);
        retryCount++;
    } else {
        retryCount = 0;
        Invidious::instance().initServers();
        qWarning() << message;
        emit error("Error loading videos");
    }
}
