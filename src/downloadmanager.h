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

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtWidgets>

class DownloadItem;
class DownloadModel;
class Video;

class DownloadManager : public QObject {

    Q_OBJECT

public:
    static DownloadManager* instance();
    void clear();
    void addItem(Video *video);
    const QVector<DownloadItem*> getItems() { return items; }
    DownloadModel* getModel() { return downloadModel; }
    DownloadItem* itemForVideo(Video *video);
    int activeItems();
    QString defaultDownloadFolder();
    QString currentDownloadFolder();

signals:
    void finished();
    void statusMessageChanged(QString status);

private slots:
    void itemFinished();
    void updateStatusMessage();
    void gotStreamUrl(QUrl url);

private:
    DownloadManager(QWidget *parent = 0);

    QVector<DownloadItem*> items;
    DownloadModel *downloadModel;

};

#endif // DOWNLOADMANAGER_H
