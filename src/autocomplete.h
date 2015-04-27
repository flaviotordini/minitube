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
#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <QtGui>

class Suggester;
class Suggestion;
class SearchLineEdit;

QT_FORWARD_DECLARE_CLASS(QListWidget)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)
QT_FORWARD_DECLARE_CLASS(QLineEdit)

class AutoComplete : public QObject {

    Q_OBJECT

public:
    AutoComplete(SearchLineEdit *buddy, QLineEdit *lineEdit);
    void setSuggester(Suggester* suggester);
    QListWidget* getPopup() { return popup; }
    void preventSuggest();
    void enableSuggest();

signals:
    void suggestionAccepted(Suggestion *suggestion);
    void suggestionAccepted(const QString &value);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    void acceptSuggestion();
    void suggest();
    void itemEntered(QListWidgetItem *item);
    void currentItemChanged(QListWidgetItem *item);
    void suggestionsReady(const QList<Suggestion*> &suggestions);
    void adjustPosition();
    void enableItemHovering();

private:
    void showSuggestions(const QList<Suggestion*> &suggestions);
    void hideSuggestions();

    SearchLineEdit *buddy;
    QLineEdit *lineEdit;
    QString originalText;
    QListWidget *popup;
    QTimer *timer;
    bool enabled;
    Suggester *suggester;
    QList<Suggestion*> suggestions;
    bool itemHovering;
};

#endif // AUTOCOMPLETE_H
