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
#include "http.h"
#include "httputils.h"

ChannelSuggest::ChannelSuggest(QObject *parent) : Suggester(parent) {

}

void ChannelSuggest::suggest(const QString &query) {
    QUrl url("https://www.youtube.com/results");
    QUrlQuery q;
    q.addQueryItem("search_type", "search_users");
    q.addQueryItem("search_query", query);
    url.setQuery(q);

    QObject *reply = HttpUtils::yt().get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void ChannelSuggest::handleNetworkData(QByteArray data) {
    const int maxSuggestions = 10;
    QStringList choices;
    choices.reserve(maxSuggestions);
    QVector<Suggestion*> suggestions;
    suggestions.reserve(maxSuggestions);

    QString html = QString::fromUtf8(data);
    static QRegularExpression re(
            "/(?:user|channel)/[a-zA-Z0-9]+[^>]+data-ytid=[\"']([^\"']+)[\"'][^>]+>([a-zA-Z0-9 "
            "]+)</a>");
    for (const auto &match : re.globalMatch(html)) {
        QString choice = match.captured(2);
        if (!choices.contains(choice, Qt::CaseInsensitive)) {
            qDebug() << match.capturedTexts();
            QString channelId = match.captured(1);
            suggestions << new Suggestion(choice, "channel", channelId);
            choices << choice;
            if (choices.size() == maxSuggestions) break;
        }
    }

    emit ready(suggestions);
}
