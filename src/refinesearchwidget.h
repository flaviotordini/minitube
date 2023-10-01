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

#ifndef REFINESEARCHWIDGET_H
#define REFINESEARCHWIDGET_H

#include <QtWidgets>

class SearchParams;

class RefineSearchWidget : public QWidget {

    Q_OBJECT

public:
    RefineSearchWidget(QWidget *parent = 0);

    bool isDirty() { return dirty; }
    void setDirty(bool dirty) { this->dirty = dirty; }
    void setSearchParams(SearchParams* params);

signals:
    void paramChanged(QString name, QVariant value);
    void searchRefined();
    void done();

private slots:
    void actionTriggered(QAction* action);
    void doneClicked();

private:
    void setup();
    void setupLabel(const QString &text, QBoxLayout *layout);
    QToolBar *setupBar(const QString &paramName);

    QHash<QString, QToolBar*> bars;
    bool dirty;
    QPushButton *doneButton;

};

#endif // REFINESEARCHWIDGET_H
