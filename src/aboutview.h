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

#ifndef ABOUTVIEW_H
#define ABOUTVIEW_H

#include <QtWidgets>

#include "view.h"
#include "constants.h"

class AboutView : public View {

    Q_OBJECT

public:
    AboutView(QWidget *parent);
    void appear();
    QHash<QString, QVariant> metadata() {
        QHash<QString, QVariant> metadata;
        metadata.insert("title", tr("About"));
        return metadata;
    }

protected:
    void paintEvent(QPaintEvent *e);

private slots:
    void screenChanged();

private:
    QLabel *logo;
    QPushButton *closeButton;

};
#endif
