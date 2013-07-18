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

#ifndef SUGGESTCOMPLETION_H
#define SUGGESTCOMPLETION_H

#include <QtGui>

class Suggester;
class SearchLineEdit;

class AutoComplete : public QObject {
    Q_OBJECT

public:
    AutoComplete(SearchLineEdit *parent, QLineEdit *editor);
    ~AutoComplete();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);
    void setSuggester(Suggester* suggester);
    QListWidget* getPopup() { return popup; }

public slots:
    void doneCompletion();
    void preventSuggest();
    void enableSuggest();
    void autoSuggest();
    void currentItemChanged(QListWidgetItem *current);
    void suggestionsReady(QStringList suggestions);

signals:
    void suggestionAccepted(const QString &suggestion);

private:
    SearchLineEdit *buddy;
    QLineEdit *editor;
    QString originalText;
    QListWidget *popup;
    QTimer *timer;
    bool enabled;
    Suggester* suggester;

};

#endif // SUGGESTCOMPLETION_H
