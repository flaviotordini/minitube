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

#include "channelsuggest.h"
#include "networkaccess.h"

namespace The {
    NetworkAccess* http();
}

ChannelSuggest::ChannelSuggest(QObject *parent) : Suggester(parent) {

}

void ChannelSuggest::suggest(const QString &query) {
    QUrl url("https://www.youtube.com/results");
#if QT_VERSION >= 0x050000
        {
            QUrl &u = url;
            QUrlQuery url;
#endif
    url.addQueryItem("search_type", "search_users");
    url.addQueryItem("search_query", query);
#if QT_VERSION >= 0x050000
            u.setQuery(url);
        }
#endif
    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void ChannelSuggest::handleNetworkData(QByteArray data) {
    QStringList choices;
    QList<Suggestion*> suggestions;

    QString html = QString::fromUtf8(data);
    QRegExp re("/(?:user|channel)/[a-zA-Z0-9]+[^>]+data-ytid=[\"']([^\"']+)[\"'][^>]+>([a-zA-Z0-9 ]+)</a>");

    int pos = 0;
    while ((pos = re.indexIn(html, pos)) != -1) {
        QString choice = re.cap(2);
        if (!choices.contains(choice, Qt::CaseInsensitive)) {
            qDebug() << re.capturedTexts();
            QString channelId = re.cap(1);
            suggestions << new Suggestion(choice, "channel", channelId);
            choices << choice;
            if (choices.size() == 10) break;
        }
        pos += re.matchedLength();
    }

    emit ready(suggestions);
}
