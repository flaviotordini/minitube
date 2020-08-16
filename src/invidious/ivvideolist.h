#ifndef IVVIDEOLIST_H
#define IVVIDEOLIST_H

#include "ivvideosource.h"
#include <QtCore>

class IVVideoList : public IVVideoSource {
    Q_OBJECT

public:
    IVVideoList(const QString &req, const QString &name, QObject *parent = nullptr);

    void reallyLoadVideos(int max, int startIndex);
    QString getName() { return name; };
    bool hasMoreVideos() { return false; }

private:
    QString name;
    QString req;
};

#endif // IVVIDEOLIST_H
