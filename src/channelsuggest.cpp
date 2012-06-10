#include "channelsuggest.h"
#include "networkaccess.h"

namespace The {
    NetworkAccess* http();
}

ChannelSuggest::ChannelSuggest(QObject *parent) : Suggester() {

}

void ChannelSuggest::suggest(QString query) {

    /* // TODO how to localize results?
    QString locale = QLocale::system().name().replace("_", "-");
    // case for system locales such as "C"
    if (locale.length() < 2) {
        locale = "en-US";
    }*/

    QUrl url("http://www.youtube.com/results?search_type=search_users");
    url.addQueryItem("search_query", query);
    // url.addQueryItem("hl", "it-IT");

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void ChannelSuggest::handleNetworkData(QByteArray data) {
    QStringList choices;
    QString html = QString::fromUtf8(data);
    QRegExp re("/user/([a-zA-Z0-9]+)");

    int pos = 0;
    while ((pos = re.indexIn(html, pos)) != -1) {
        // qDebug() << re.cap(0) << re.cap(1);
        QString choice = re.cap(1);
        if (!choices.contains(choice, Qt::CaseInsensitive)) {
            choices << choice;
            if (choices.size() == 10) break;
        }
        pos += re.matchedLength();
    }

    emit ready(choices);
}
