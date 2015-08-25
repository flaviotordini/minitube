/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#ifndef VIDEOMIMEDATA_H
#define VIDEOMIMEDATA_H

#include <QMimeData>
#include "video.h"

class VideoMimeData : public QMimeData {

    Q_OBJECT

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
