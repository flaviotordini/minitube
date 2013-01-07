#ifndef YTREGIONS_H
#define YTREGIONS_H

#include <QtGui>

struct YTRegion {
    QString id;
    QString name;
    bool operator<(const YTRegion &other) const {
        return name < other.name;
    }
};

class YTRegions : public QObject {

public:
    static const QList<YTRegion> & list();
    static const YTRegion & localRegion();
    static const YTRegion & worldwideRegion();
    static void setRegion(QString regionId);
    static QString currentRegionId();
    static YTRegion currentRegion();
    static QIcon iconForRegionId(QString regionId);

private:
    static YTRegion r(QString name, QString id);
    static YTRegion regionById(QString id);
    YTRegions();

};

#endif // YTREGIONS_H
