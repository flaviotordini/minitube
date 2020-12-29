#ifndef IVVIDEOSOURCE_H
#define IVVIDEOSOURCE_H

#include <QtCore>

#include "invidious.h"
#include "videosource.h"

class IVVideoSource : public VideoSource {
    Q_OBJECT

public:
    IVVideoSource(QObject *parent = nullptr) : VideoSource(parent) {}

    void loadVideos(int max, int startIndex);
    void abort();

    virtual void reallyLoadVideos(int max, int startIndex) = 0;

protected slots:
    void handleError(QString message);

protected:
    bool aborted = false;

private:
    int retryCount = 0;
    int max = 0;
    int startIndex = 0;
};

#endif // IVVIDEOSOURCE_H
