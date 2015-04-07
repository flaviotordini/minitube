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
#ifdef APP_YT3
#include "datautils.h"
#include "yt3.h"
#include "ytregions.h"
#include <QtScript>
#endif

namespace The {
NetworkAccess* http();
}

YTCategories::YTCategories(QObject *parent) : QObject(parent) { }

void YTCategories::loadCategories(QString language) {
    if (language.isEmpty())
        language = QLocale::system().uiLanguages().first();
    lastLanguage = language;

#ifdef APP_YT3
    QUrl url = YT3::instance().method("videoCategories");

#if QT_VERSION >= 0x050000
    {
        QUrl &u = url;
        QUrlQuery url(u);
#endif

        url.addQueryItem("part", "snippet");
        url.addQueryItem("hl", language);

        QString regionCode = YTRegions::currentRegionId();
        if (regionCode.isEmpty()) regionCode = "us";
        url.addQueryItem("regionCode", regionCode);

#if QT_VERSION >= 0x050000
        u.setQuery(url);
    }
#endif

#else
    QString url = "http://gdata.youtube.com/schemas/2007/categories.cat?hl=" + language;
#endif


    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseCategories(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

#ifdef APP_YT3

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

#else

void YTCategories::parseCategories(QByteArray bytes) {
    QList<YTCategory> categories;

    QXmlStreamReader xml(bytes);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("category")) {
            QString term = xml.attributes().value("term").toString();
            QString label = xml.attributes().value("label").toString();
            while(xml.readNextStartElement())
                if (xml.name() == QLatin1String("assignable")) {
                    YTCategory category;
                    category.term = term;
                    category.label = label;
                    categories << category;
                } else xml.skipCurrentElement();
        }
    }

    if (xml.hasError()) {
        emit error(xml.errorString());
        return;
    }

    emit categoriesLoaded(categories);
}

#endif

void YTCategories::requestError(QNetworkReply *reply) {
    if (lastLanguage != "en") loadCategories("en");
    else emit error(reply->errorString());
}
