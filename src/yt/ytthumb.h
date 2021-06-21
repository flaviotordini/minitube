#ifndef YTTHUMB_H
#define YTTHUMB_H

#include <QtCore>

#include "variantpromise.h"

class YTThumb {
public:
    YTThumb() {} // needed by QVector
    YTThumb(int width, int height, const QString &url);
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    const QString &getUrl() const { return url; }

    VariantPromise &load(QObject *parent);

private:
    int width;
    int height;
    QString url;
    VariantPromise *promise = nullptr;
};

#endif // YTTHUMB_H
