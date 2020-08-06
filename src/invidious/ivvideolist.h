#ifndef IVVIDEOLIST_H
#define IVVIDEOLIST_H

#include "videosource.h"
#include <QtCore>

class IVVideoList : public VideoSource {
    Q_OBJECT

public:
    IVVideoList(const QString &req, const QString &name, QObject *parent = nullptr);

    void loadVideos(int max, int startIndex);
    void abort();
    QString getName() { return name; };
    bool hasMoreVideos() { return false; }

private:
    bool aborted;
    QString name;
    QString req;
};

#endif // IVVIDEOLIST_H
