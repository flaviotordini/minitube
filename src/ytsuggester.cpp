#include "youtubesuggest.h"
#include <QtXml>
#include "networkaccess.h"

#define GSUGGEST_URL "http://suggestqueries.google.com/complete/search?ds=yt&output=toolbar&hl=%1&q=%2"

namespace The {
    NetworkAccess* http();
}

YouTubeSuggest::YouTubeSuggest(QObject *parent) : Suggester() {

}

void YouTubeSuggest::suggest(QString query) {
    QString locale = QLocale::system().name().replace("_", "-");
    // case for system locales such as "C"
    if (locale.length() < 2) {
        locale = "en-US";
    }

    QString url = QString(GSUGGEST_URL).arg(locale, query);

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void YouTubeSuggest::handleNetworkData(QByteArray response) {
    QStringList choices;

    QXmlStreamReader xml(response);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "suggestion") {
                QStringRef str = xml.attributes().value("data");
                choices << str.toString();
            }
        }
    }
    emit ready(choices);
}
