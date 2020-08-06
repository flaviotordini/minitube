#ifndef IVCHANNELSOURCE_H
#define IVCHANNELSOURCE_H

#include "videosource.h"
#include <QtNetwork>

class SearchParams;

class IVChannelSource : public VideoSource {
    Q_OBJECT

public:
    IVChannelSource(SearchParams *searchParams, QObject *parent = nullptr);

    void loadVideos(int max, int startIndex);
    void abort();
    QString getName();
    const QList<QAction *> &getActions();
    int maxResults();

    SearchParams *getSearchParams() const { return searchParams; }

private:
    SearchParams *searchParams;
    bool aborted;
    QString name;
};

#endif // IVCHANNELSOURCE_H
