#ifndef YTSEARCH_H
#define YTSEARCH_H

#include <QtNetwork>
#include "videosource.h"

class SearchParams;
class Video;

class YTSearch : public VideoSource {

    Q_OBJECT

public:
    YTSearch(SearchParams *params, QObject *parent = 0);
    void loadVideos(int max, int skip);
    virtual void abort();
    virtual const QStringList & getSuggestions();
    static QString videoIdFromUrl(QString url);
    QString getName();
    SearchParams* getSearchParams() const { return searchParams; }

    bool operator==(const YTSearch &other) const {
        return searchParams == other.getSearchParams();
    }

private slots:
    void parseResults(QByteArray data);
    void requestError(QNetworkReply *reply);

private:
    SearchParams *searchParams;
    bool aborted;
    QStringList suggestions;
    QString name;
};

#endif // YTSEARCH_H
