#ifndef IVSEARCH_H
#define IVSEARCH_H

#include "videosource.h"
#include <QtNetwork>

class SearchParams;
class Video;

class IVSearch : public VideoSource {
    Q_OBJECT

public:
    IVSearch(SearchParams *params, QObject *parent = 0);
    void loadVideos(int max, int startIndex);
    void abort();
    QString getName();
    const QList<QAction *> &getActions();
    int maxResults();
    SearchParams *getSearchParams() const { return searchParams; }

private slots:
    void parseResults(const QByteArray &data);

private:
    SearchParams *searchParams;
    bool aborted;
    QString name;
};

#endif // IVSEARCH_H
