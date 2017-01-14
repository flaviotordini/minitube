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

#ifndef SNAPSHOTPREVIEW_H
#define SNAPSHOTPREVIEW_H

#include <QtWidgets>

#ifdef APP_PHONON
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>
#endif

class SnapshotPreview : public QWidget {

    Q_OBJECT

public:
    SnapshotPreview(QWidget *parent = 0);
    void start(QWidget *widget, const QPixmap &pixmap, bool soundOnly);

signals:
    void done();

protected:
    void paintEvent(QPaintEvent *e);

private slots:
    void finish();

private:
    QPixmap pixmap;
    QTimeLine *timeLine;
    QPoint offset;
    QTimer *timer;
#ifdef APP_PHONON
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
#endif
};

#endif // SNAPSHOTPREVIEW_H
