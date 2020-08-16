#ifndef IVCHANNELSOURCE_H
#define IVCHANNELSOURCE_H

#include "ivvideosource.h"
#include <QtNetwork>

class SearchParams;

class IVChannelSource : public IVVideoSource {
    Q_OBJECT

public:
    IVChannelSource(SearchParams *searchParams, QObject *parent = nullptr);

    void reallyLoadVideos(int max, int startIndex);
    QString getName();
    const QList<QAction *> &getActions();
    int maxResults();

    SearchParams *getSearchParams() const { return searchParams; }

private:
    SearchParams *searchParams;
    QString name;
};

#endif // IVCHANNELSOURCE_H
