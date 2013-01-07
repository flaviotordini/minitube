#include "videosource.h"

void VideoSource::setParam(QString name, QVariant value) {
    bool success = setProperty(name.toUtf8(), value);
    if (!success) qWarning() << "Failed to set property" << name << value.toString();
}
