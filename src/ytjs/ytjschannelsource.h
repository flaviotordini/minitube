#ifndef YTJSCHANNELSOURCE_H
#define YTJSCHANNELSOURCE_H

#include "videosource.h"

class SearchParams;
class Video;

class YTJSChannelSource : public VideoSource {
    Q_OBJECT

public:
    YTJSChannelSource(SearchParams *searchParams, QObject *parent = 0);
    void loadVideos(int max, int startIndex);
    bool hasMoreVideos() { return !continuation.isEmpty(); }
    void abort() { aborted = true; }
    QString getName();
    const QList<QAction *> &getActions();
    SearchParams *getSearchParams() const { return searchParams; }

private:
    SearchParams *searchParams;
    bool aborted = false;
    QString name;

    QString continuation;
};

#endif // YTJSCHANNELSOURCE_H
