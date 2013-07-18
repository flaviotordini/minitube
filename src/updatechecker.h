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

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QXmlStreamReader>
#include <QNetworkReply>

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    UpdateChecker();
    void checkForUpdate();
    QString remoteVersion();

signals:
    void newVersion(QString);

private slots:
    void requestFinished(QByteArray);

private:

    bool m_needUpdate;
    QString m_remoteVersion;
    QNetworkReply *networkReply;

};

class UpdateCheckerStreamReader : public QXmlStreamReader {

public:
    bool read(QByteArray data);
    QString remoteVersion();
    bool needUpdate() { return m_needUpdate; }

private:
    QString m_remoteVersion;
    bool m_needUpdate;

};

#endif // UPDATECHECKER_H
