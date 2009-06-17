#ifndef VIDEOMIMEDATA_H
#define VIDEOMIMEDATA_H

#include <QMimeData>
#include "video.h"

class VideoMimeData : public QMimeData {

public:
    VideoMimeData();

    virtual QStringList formats() const;
    virtual bool hasFormat( const QString &mimeType ) const;

    QList<Video*> videos() const { return m_videos; }

    void addVideo(Video *video) {
        m_videos << video;
    }

private:
    QList<Video*> m_videos;

};

#endif // VIDEOMIMEDATA_H
