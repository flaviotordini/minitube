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
#include "networkaccess.h"
#include "datautils.h"
#include "yt3.h"
#include "ytregions.h"
#include <QtScript>

namespace The {
NetworkAccess* http();
}

YTCategories::YTCategories(QObject *parent) : QObject(parent) { }

void YTCategories::loadCategories(QString language) {
    if (language.isEmpty())
        language = QLocale::system().uiLanguages().first();
    lastLanguage = language;

    QUrl url = YT3::instance().method("videoCategories");

    QUrlQuery q(url);
    q.addQueryItem("part", "snippet");
    q.addQueryItem("hl", language);

    QString regionCode = YTRegions::currentRegionId();
    if (regionCode.isEmpty()) regionCode = "us";
    q.addQueryItem("regionCode", regionCode);
    url.setQuery(q);

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseCategories(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTCategories::parseCategories(QByteArray bytes) {
    QList<YTCategory> categories;

    QScriptEngine engine;
    QScriptValue json = engine.evaluate("(" + QString::fromUtf8(bytes) + ")");

    QScriptValue items = json.property("items");

    if (items.isArray()) {
        QScriptValueIterator it(items);
        while (it.hasNext()) {
            it.next();
            QScriptValue item = it.value();
            // For some reason the array has an additional element containing its size.
            if (!item.isObject()) continue;

            QScriptValue snippet = item.property("snippet");

            bool isAssignable = snippet.property("assignable").toBool();
            if (!isAssignable) continue;

            YTCategory category;
            category.term = item.property("id").toString();
            category.label = snippet.property("title").toString();
            categories << category;
        }
    }

    emit categoriesLoaded(categories);
}

void YTCategories::requestError(QNetworkReply *reply) {
    if (lastLanguage != "en") loadCategories("en");
    else emit error(reply->errorString());
}
