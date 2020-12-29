#ifndef IVSEARCH_H
#define IVSEARCH_H

#include "ivvideosource.h"
#include <QtNetwork>

class SearchParams;
class Video;

class IVSearch : public IVVideoSource {
    Q_OBJECT

public:
    IVSearch(SearchParams *params, QObject *parent = 0);
    void reallyLoadVideos(int max, int startIndex);
    QString getName();
    const QList<QAction *> &getActions();
    int maxResults();
    SearchParams *getSearchParams() const { return searchParams; }

private slots:
    void parseResults(const QByteArray &data);

private:
    SearchParams *searchParams;
    QString name;
};

#endif // IVSEARCH_H
