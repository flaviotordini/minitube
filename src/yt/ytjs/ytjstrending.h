#ifndef YTJSTRENDING_H
#define YTJSTRENDING_H

#include "videosource.h"

class Video;

class YTJSTrending : public VideoSource {
    Q_OBJECT

public:
    YTJSTrending(QString name, QVariantMap params, QObject *parent = 0);
    void loadVideos(int max, int startIndex);
    void abort() { aborted = true; }
    QString getName() { return name; }

private:
    const QString name;
    const QVariantMap params;
    bool aborted = false;
};

#endif // YTJSTRENDING_H
