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

#include "ytcategories.h"
#include "http.h"
#include "httputils.h"
#include "datautils.h"
#include "yt3.h"
#include "ytregions.h"

YTCategories::YTCategories(QObject *parent) : QObject(parent) { }

void YTCategories::loadCategories(QString language) {
    if (language.isEmpty()) {
        language = QLocale::system().uiLanguages().first();
        int index = language.indexOf('-');
        if (index > 0) language = language.mid(0, index);
    }
    lastLanguage = language;

    QUrl url = YT3::instance().method("videoCategories");

    QUrlQuery q(url);
    q.addQueryItem("part", "snippet");
    q.addQueryItem("hl", language);

    QString regionCode = YTRegions::currentRegionId();
    if (regionCode.isEmpty()) regionCode = "us";
    q.addQueryItem("regionCode", regionCode);
    url.setQuery(q);

    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseCategories(QByteArray)));
    connect(reply, SIGNAL(error(QString)), SLOT(requestError(QString)));
}

void YTCategories::parseCategories(QByteArray bytes) {
    QVector<YTCategory> categories;

    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    QJsonObject obj = doc.object();
    QJsonArray items = obj["items"].toArray();
    categories.reserve(items.size());
    foreach (const QJsonValue &v, items) {
        QJsonObject item = v.toObject();
        QJsonObject snippet = item["snippet"].toObject();
        bool isAssignable = snippet["assignable"].toBool();
        if (!isAssignable) continue;

        YTCategory category;
        category.term = item["id"].toString();
        category.label = snippet["title"].toString();
        if (category.label.startsWith(QLatin1String("News"))) continue;
        categories << category;
    }

    emit categoriesLoaded(categories);
}

void YTCategories::requestError(const QString &message) {
    if (lastLanguage != "en") loadCategories("en");
    else emit error(message);
}
