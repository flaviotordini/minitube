#ifndef YOUTUBESEARCH_H
#define YOUTUBESEARCH_H

#include "video.h"
#include "searchparams.h"

class YouTubeSearch : public QObject {

    Q_OBJECT

public:
    YouTubeSearch();
    void search(SearchParams *searchParams, int max, int skip);
    void abort();
    QList<Video*> getResults();

signals:
    void gotVideo(Video*);
    void finished(int total);

private slots:
    void parseResults(QByteArray data);

private:

    QList<Video*> videos;

    bool abortFlag;

};

#endif // YOUTUBESEARCH_H
