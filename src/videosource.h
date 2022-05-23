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

#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QAction>
#include <QtCore>

#include "video.h"

class VideoSource : public QObject {
    Q_OBJECT

public:
    VideoSource(QObject *parent = 0) : QObject(parent) {}
    virtual void loadVideos(int max, int startIndex) = 0;
    virtual bool hasMoreVideos() { return true; }
    virtual void abort() = 0;
    virtual QString getName() = 0;
    virtual const QList<QAction *> &getActions() {
        static const QList<QAction *> noActions;
        return noActions;
    }
    virtual int maxResults() { return 0; }

public slots:
    void setParam(const QString &name, const QVariant &value);

signals:
    void gotVideos(const QVector<Video *> &videos);
    void finished(int total);
    void error(QString message);
    void nameChanged(QString name);
};

#endif // VIDEOSOURCE_H
