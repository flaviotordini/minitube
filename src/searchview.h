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

#ifndef __SEARCHVIEW_H__
#define __SEARCHVIEW_H__

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
    SearchView(QWidget *parent = 0);
    void updateRecentKeywords();
    void updateRecentChannels();

public slots:
    void appear();
    void disappear();
    void watch(const QString &query);
    void watchChannel(const QString &channelId);
    void watchKeywords(const QString &query);

signals:
    void search(SearchParams*);

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void watch();
    void textChanged(const QString &text);
    void searchTypeChanged(int index);
    void suggestionAccepted(Suggestion *suggestion);
    void screenChanged();
    void onChannelSuggestions(const QList<Suggestion*> &suggestions);

private:
    YTSuggester *youtubeSuggest;
    ChannelSuggest *channelSuggest;

    QComboBox *typeCombo;
    SearchWidget *queryEdit;
    QLabel *recentKeywordsLabel;
    QBoxLayout *recentKeywordsLayout;
    QLabel *recentChannelsLabel;
    QBoxLayout *recentChannelsLayout;
    QLabel *message;
    QPushButton *watchButton;

    QStringList recentKeywords;
    QStringList recentChannels;

    QList<Suggestion*> lastChannelSuggestions;

    ClickableLabel *logo;
};

#endif // __SEARCHVIEW_H__
