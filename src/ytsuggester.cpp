#include "ytsuggester.h"
#include <QtXml>
#include "networkaccess.h"

#define GSUGGEST_URL "http://suggestqueries.google.com/complete/search?ds=yt&output=toolbar&hl=%1&q=%2"

namespace The {
    NetworkAccess* http();
}

YTSuggester::YTSuggester(QObject *parent) : Suggester() {

}

void YTSuggester::suggest(QString query) {
    if (query.startsWith("http")) return;

#if QT_VERSION >= 0x040800
    QString locale = QLocale::system().uiLanguages().first();
#else
    QString locale = QLocale::system().name().replace("_", "-");
#endif

    // case for system locales such as "C"
    if (locale.length() < 2) {
        locale = "en-US";
    }

    QString url = QString(GSUGGEST_URL).arg(locale, query);

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void YTSuggester::handleNetworkData(QByteArray response) {
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
