#include "videomimedata.h"

VideoMimeData::VideoMimeData() {}

QStringList VideoMimeData::formats() const {
    QStringList formats( QMimeData::formats() );
    formats.append("application/x-minitube-video");
    return formats;
}

bool VideoMimeData::hasFormat( const QString &mimeType ) const {
    return mimeType == QLatin1String("application/x-minitube-video");
}
