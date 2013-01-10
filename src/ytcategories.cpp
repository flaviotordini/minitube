#include "ytcategories.h"
#include "networkaccess.h"
#include <QtXml>

namespace The {
NetworkAccess* http();
}

YTCategories::YTCategories(QObject *parent) : QObject(parent) { }

void YTCategories::loadCategories(QString language) {
    if (language.isEmpty())
        language = QLocale::system().uiLanguages().first();
    lastLanguage = language;

    QString url = "http://gdata.youtube.com/schemas/2007/categories.cat?hl=" + language;
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseCategories(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

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

void YTCategories::requestError(QNetworkReply *reply) {
    if (lastLanguage != "en") loadCategories("en");
    else emit error(reply->errorString());
}
