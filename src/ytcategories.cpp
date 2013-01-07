#include "ytcategories.h"
#include "networkaccess.h"
#include <QtXml>

namespace The {
NetworkAccess* http();
}

YTCategories::YTCategories(QObject *parent) : QObject(parent) { }

void YTCategories::loadCategories() {
    QString url = "http://gdata.youtube.com/schemas/2007/categories.cat?hl=";
#if QT_VERSION >= 0x040800
    url += QLocale::system().uiLanguages().first();
#else
    url += QLocale::system().name().replace('_', '-');
#endif
    qDebug() << url;
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(parseCategories(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(requestError(QNetworkReply*)));
}

void YTCategories::parseCategories(QByteArray bytes) {
    QList<YTCategory> categories;

    QXmlStreamReader xml(bytes);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "category") {
            QString term = xml.attributes().value("term").toString();
            QString label = xml.attributes().value("label").toString();
            while(xml.readNextStartElement())
                if (xml.name() == "assignable") {
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
    emit error(reply->errorString());
}
