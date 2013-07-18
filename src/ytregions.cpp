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

YTRegions::YTRegions() : QObject() { }

const QList<YTRegion> & YTRegions::list() {
    static QList<YTRegion> list;
    if (list.isEmpty()) {
        list << r(tr("Algeria"), "DZ")
             << r(tr("Argentina"), "AR")
             << r(tr("Australia"), "AU")
             << r(tr("Belgium"), "BE")
             << r(tr("Brazil"), "BR")
             << r(tr("Canada"), "CA")
             << r(tr("Chile"), "CL")
             << r(tr("Colombia"), "CO")
             << r(tr("Czech Republic"), "CZ")
             << r(tr("Egypt"), "EG")
             << r(tr("France"), "FR")
             << r(tr("Germany"), "DE")
             << r(tr("Ghana"), "GH")
             << r(tr("Greece"), "GR")
             << r(tr("Hong Kong"), "HK")
             << r(tr("Hungary"), "HU")
             << r(tr("India"), "IN")
             << r(tr("Indonesia"), "ID")
             << r(tr("Ireland"), "IE")
             << r(tr("Israel"), "IL")
             << r(tr("Italy"), "IT")
             << r(tr("Japan"), "JP")
             << r(tr("Jordan"), "JO")
             << r(tr("Kenya"), "KE")
             << r(tr("Malaysia"), "MY")
             << r(tr("Mexico"), "MX")
             << r(tr("Morocco"), "MA")
             << r(tr("Netherlands"), "NL")
             << r(tr("New Zealand"), "NZ")
             << r(tr("Nigeria"), "NG")
             << r(tr("Peru"), "PE")
             << r(tr("Philippines"), "PH")
             << r(tr("Poland"), "PL")
             << r(tr("Russia"), "RU")
             << r(tr("Saudi Arabia"), "SA")
             << r(tr("Singapore"), "SG")
             << r(tr("South Africa"), "ZA")
             << r(tr("South Korea"), "KR")
             << r(tr("Spain"), "ES")
             << r(tr("Sweden"), "SE")
             << r(tr("Taiwan"), "TW")
             << r(tr("Tunisia"), "TN")
             << r(tr("Turkey"), "TR")
             << r(tr("Uganda"), "UG")
             << r(tr("United Arab Emirates"), "AE")
             << r(tr("United Kingdom"), "GB")
             << r(tr("Yemen"), "YE");
/*
        list << r(QLocale::Algeria, "DZ")
             << r(QLocale::Argentina, "AR")
             << r(QLocale::Australia, "AU")
             << r(QLocale::Belgium, "BE")
             << r(QLocale::Brazil, "BR")
             << r(QLocale::Canada, "CA")
             << r(QLocale::Chile, "CL")
             << r(QLocale::Colombia, "CO")
             << r(QLocale::CzechRepublic, "CZ")
             << r(QLocale::Egypt, "EG")
             << r(QLocale::France, "FR")
             << r(QLocale::Germany, "DE")
             << r(QLocale::Ghana, "GH")
             << r(QLocale::Greece, "GR")
             << r(QLocale::HongKong, "HK")
             << r(QLocale::Hungary, "HU")
             << r(QLocale::India, "IN")
             << r(QLocale::Indonesia, "ID")
             << r(QLocale::Ireland, "IE")
             << r(QLocale::Israel, "IL")
             << r(QLocale::Italy, "IT")
             << r(QLocale::Japan, "JP")
             << r(QLocale::Jordan, "JO")
             << r(QLocale::Kenya, "KE")
             << r(QLocale::Malaysia, "MY")
             << r(QLocale::Mexico, "MX")
             << r(QLocale::Morocco, "MA")
             << r(QLocale::Netherlands, "NL")
             << r(QLocale::NewZealand, "NZ")
             << r(QLocale::Nigeria, "NG")
             << r(QLocale::Peru, "PE")
             << r(QLocale::Philippines, "PH")
             << r(QLocale::Poland, "PL")
             << r(QLocale::RussianFederation, "RU")
             << r(QLocale::SaudiArabia, "SA")
             << r(QLocale::Singapore, "SG")
             << r(QLocale::SouthAfrica, "ZA")
             << r(QLocale::RepublicOfKorea, "KR")
             << r(QLocale::Spain, "ES")
             << r(QLocale::Sweden, "SE")
             << r(QLocale::Taiwan, "TW")
             << r(QLocale::Tunisia, "TN")
             << r(QLocale::Turkey, "TR")
             << r(QLocale::Uganda, "UG")
             << r(QLocale::UnitedArabEmirates, "AE")
             << r(QLocale::UnitedKingdom, "GB")
             << r(QLocale::Yemen, "YE");
             */
        qSort(list);
    }
    return list;
}

YTRegion YTRegions::r(QString name, QString id) {
    YTRegion r = {id, name};
    return r;
}

const YTRegion & YTRegions::localRegion() {
    static YTRegion region;
    if (region.name.isEmpty()) {
        QString country = QLocale::system().name().right(2);
        foreach (YTRegion r, list())
            if (r.id == country) {
                region = r;
                break;
            } // else qDebug() << r.id << country;
    }
    return region;
}

const YTRegion & YTRegions::worldwideRegion() {
    static YTRegion region = {"", tr("Worldwide")};
    return region;
}

void YTRegions::setRegion(QString regionId) {
    QSettings settings;
    settings.setValue("regionId", regionId);
}

QString YTRegions::currentRegionId() {
    QSettings settings;
    return settings.value("regionId").toString();
}

YTRegion YTRegions::currentRegion() {
    return regionById(currentRegionId());
}

YTRegion YTRegions::regionById(QString id) {
    if (id.isEmpty()) return worldwideRegion();
    YTRegion region;
    foreach (YTRegion r, list())
        if (r.id == id) {
            region = r;
            break;
        }
    if (region.name.isEmpty()) return worldwideRegion();
    return region;
}

QIcon YTRegions::iconForRegionId(QString regionId) {
    if (regionId.isEmpty()) return QIcon(":images/worldwide.png");
    return QIcon(":flags/" + regionId.toLower() + ".png");
}
