#ifndef SEARCHVIDEOSOURCE_H
#define SEARCHVIDEOSOURCE_H

#include <QObject>
#include <videosource.h>

class SearchParams;

class SearchVideoSource : public VideoSource {
    Q_OBJECT

public:
    SearchVideoSource(SearchParams *searchParams, QObject *parent = 0);

    SearchParams *getSearchParams() const { return searchParams; }

    void loadVideos(int max, int startIndex);
    bool hasMoreVideos();
    void abort() { aborted = true; }
    QString getName();
    const QList<QAction *> &getActions();
    int maxResults();

private:
    void connectSource(int max, int startIndex);

    SearchParams *searchParams;
    VideoSource *source = nullptr;
    bool aborted = false;
};

#endif // SEARCHVIDEOSOURCE_H
