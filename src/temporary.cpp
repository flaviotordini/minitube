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

#include "temporary.h"
#include "constants.h"

static QVector<QString> paths;
#ifdef APP_LINUX
static QString userName;
#endif

Temporary::Temporary() { }

QString Temporary::filename() {
    static const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString tempFile = tempDir + "/" + Constants::UNIX_NAME + "-" +
                       QString::number(QRandomGenerator::global()->generate());

#ifdef APP_LINUX
    if (userName.isNull()) {
        userName = QString(getenv("USERNAME"));
        if (userName.isEmpty())
            userName = QString(getenv("USER"));
    }
    if (!userName.isEmpty())
        tempFile += "-" + userName;
#endif

    // tempFile += ".mp4";

    if (QFile::exists(tempFile) && !QFile::remove(tempFile)) {
        qDebug() << "Cannot remove temp file" << tempFile;
    }

    paths << tempFile;

    if (paths.size() > 1) {
        QString removedFile = paths.takeFirst();
        if (QFile::exists(removedFile) && !QFile::remove(removedFile)) {
            qDebug() << "Cannot remove temp file" << removedFile;
        }
    }

    return tempFile;

}

void Temporary::deleteAll() {
    foreach(const QString &path, paths) {
        if (QFile::exists(path) && !QFile::remove(path)) {
            qDebug() << "Cannot remove temp file" << path;
        }
    }
}
