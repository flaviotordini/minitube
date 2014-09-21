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

#ifndef SNAPSHOTSETTINGS_H
#define SNAPSHOTSETTINGS_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

class SnapshotSettings : public QWidget {

    Q_OBJECT

public:
    SnapshotSettings(QWidget *parent = 0);
    void setSnapshot(const QPixmap &pixmap, const QString &filename);

    static void setCurrentLocation(const QString &location);
    static QString getCurrentLocation();
    static QString displayPath(const QString &path);

private slots:
    void changeFolder();
    void folderChosen(const QString &folder);
    void showFile();
    void openFile();

private:
    QToolButton *thumb;
    QLabel *message;
    QPushButton *changeFolderButton;
    QString filename;

};

#endif // SNAPSHOTSETTINGS_H
