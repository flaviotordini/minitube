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

#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <QtWidgets>

#include "view.h"

class SearchWidget;
class SearchParams;
class YTSuggester;
class ChannelSuggest;
class Suggestion;
class ClickableLabel;

class SearchView : public View {
    Q_OBJECT

public:
    SearchView(QWidget *parent = nullptr);
    void updateRecentKeywords();
    void updateRecentChannels();

public slots:
    void appear();
    void disappear();
    void watch(const QString &query);
    void watchChannel(const QString &channelId);
    void watchKeywords(const QString &query);

signals:
    void search(SearchParams *);

private slots:
    void watch();
    void textChanged(const QString &text);
    void searchTypeChanged(int index);
    void suggestionAccepted(Suggestion *suggestion);
    void onChannelSuggestions(const QVector<Suggestion *> &suggestions);

private:
    YTSuggester *youtubeSuggest;
    ChannelSuggest *channelSuggest;

    SearchWidget *queryEdit;
    QLabel *recentKeywordsLabel;
    QBoxLayout *recentKeywordsLayout;
    QLabel *recentChannelsLabel;
    QBoxLayout *recentChannelsLayout;

    QStringList recentKeywords;
    QStringList recentChannels;

    QVector<Suggestion *> lastChannelSuggestions;

    ClickableLabel *logo;
};

#endif // SEARCHVIEW_H
