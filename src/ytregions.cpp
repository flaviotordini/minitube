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

#include "ytregions.h"
#include "iconutils.h"

YTRegions::YTRegions() : QObject() {}

const QVector<YTRegion> &YTRegions::list() {
    static const QVector<YTRegion> list = [] {
        QVector<YTRegion> l = {r(tr("Algeria"), "DZ"),
                               r(tr("Argentina"), "AR"),
                               r(tr("Australia"), "AU"),
                               r(tr("Belgium"), "BE"),
                               r(tr("Brazil"), "BR"),
                               r(tr("Canada"), "CA"),
                               r(tr("Chile"), "CL"),
                               r(tr("Colombia"), "CO"),
                               r(tr("Czech Republic"), "CZ"),
                               r(tr("Egypt"), "EG"),
                               r(tr("France"), "FR"),
                               r(tr("Germany"), "DE"),
                               r(tr("Ghana"), "GH"),
                               r(tr("Greece"), "GR"),
                               r(tr("Hong Kong"), "HK"),
                               r(tr("Hungary"), "HU"),
                               r(tr("India"), "IN"),
                               r(tr("Indonesia"), "ID"),
                               r(tr("Ireland"), "IE"),
                               r(tr("Israel"), "IL"),
                               r(tr("Italy"), "IT"),
                               r(tr("Japan"), "JP"),
                               r(tr("Jordan"), "JO"),
                               r(tr("Kenya"), "KE"),
                               r(tr("Malaysia"), "MY"),
                               r(tr("Mexico"), "MX"),
                               r(tr("Morocco"), "MA"),
                               r(tr("Netherlands"), "NL"),
                               r(tr("New Zealand"), "NZ"),
                               r(tr("Nigeria"), "NG"),
                               r(tr("Peru"), "PE"),
                               r(tr("Philippines"), "PH"),
                               r(tr("Poland"), "PL"),
                               r(tr("Russia"), "RU"),
                               r(tr("Saudi Arabia"), "SA"),
                               r(tr("Singapore"), "SG"),
                               r(tr("South Africa"), "ZA"),
                               r(tr("South Korea"), "KR"),
                               r(tr("Spain"), "ES"),
                               r(tr("Sweden"), "SE"),
                               r(tr("Taiwan"), "TW"),
                               r(tr("Tunisia"), "TN"),
                               r(tr("Turkey"), "TR"),
                               r(tr("Uganda"), "UG"),
                               r(tr("United Arab Emirates"), "AE"),
                               r(tr("United Kingdom"), "GB"),
                               r(tr("Yemen"), "YE")};
        std::sort(l.begin(), l.end());
        return l;
    }();

    return list;
}

YTRegion YTRegions::r(const QString &name, const QString &id) {
    YTRegion r = {id, name};
    return r;
}

const YTRegion &YTRegions::localRegion() {
    static const YTRegion region = [] {
        QString country = QLocale::system().name().right(2);
        for (const YTRegion &r : list()) {
            if (r.id == country) return r;
        }
        return YTRegion();
    }();
    return region;
}

const YTRegion &YTRegions::worldwideRegion() {
    static const YTRegion region = {"", tr("Worldwide")};
    return region;
}

void YTRegions::setRegion(const QString &regionId) {
    QSettings settings;
    settings.setValue("regionId", regionId);
}

QString YTRegions::currentRegionId() {
    QSettings settings;
    return settings.value("regionId").toString();
}

const YTRegion &YTRegions::currentRegion() {
    return regionById(currentRegionId());
}

const YTRegion &YTRegions::regionById(const QString &id) {
    if (id.isEmpty()) return worldwideRegion();
    for (const YTRegion &r : list()) {
        if (r.id == id) return r;
    }
    return worldwideRegion();
}

QIcon YTRegions::iconForRegionId(const QString &regionId) {
    if (regionId.isEmpty()) return IconUtils::icon("worldwide");
    return QIcon(":flags/" + regionId.toLower() + ".png");
}
