#ifndef YTJSSEARCH_H
#define YTJSSEARCH_H

#include "videosource.h"

class SearchParams;
class Video;

class YTJSSearch : public VideoSource {
    Q_OBJECT

public:
    YTJSSearch(SearchParams *searchParams, QObject *parent = 0);
    void loadVideos(int max, int startIndex);
    void abort() { aborted = true; }
    QString getName();
    const QList<QAction *> &getActions();
    SearchParams *getSearchParams() const { return searchParams; }

private:
    SearchParams *searchParams;
    bool aborted = false;
    QString name;

    QString nextpageRef;
};

#endif // YTJSSEARCH_H
