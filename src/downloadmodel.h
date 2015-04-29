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

#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractListModel>

class DownloadManager;

class DownloadModel : public QAbstractListModel {

    Q_OBJECT

public:
    DownloadModel(DownloadManager *downloadManager, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    friend class DownloadManager;
    void setHoveredRow(int row);

public slots:
    void clearHover();
    void enterPlayIconHover();
    void exitPlayIconHover();
    void enterPlayIconPressed();
    void exitPlayIconPressed();
    void sendReset();
    void updatePlayIcon();

private:
    int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return 1; }

    DownloadManager *const downloadManager;
    int hoveredRow;
    bool playIconHovered;
    bool playIconPressed;

};

#endif // DOWNLOADMODEL_H
