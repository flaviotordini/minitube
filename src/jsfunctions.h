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

#ifndef JSFUNCTIONS_H
#define JSFUNCTIONS_H

#include <QtCore>
#include <QtScript>
#include <QtNetwork>

class JsFunctions : public QObject {

    Q_OBJECT

public:
    static JsFunctions* instance();

    // Specialized functions
    // TODO move to subclass
    QString decryptSignature(const QString &s);
    QString decryptAgeSignature(const QString &s);
    QString videoIdRE();
    QString videoTokenRE();
    QString videoInfoFmtMapRE();
    QString webPageFmtMapRE();
    QString ageGateRE();
    QString jsPlayerRE();
    QString signatureFunctionNameRE();

protected:
    QString evaluate(const QString &js);

private slots:
    void gotJs(QByteArray bytes);
    void errorJs(QNetworkReply *reply);

private:
    JsFunctions(QObject *parent = 0);
    static const QLatin1String &jsFilename();
    static const QString &jsPath();
    void loadJs();
    void parseJs(const QString &js);

    QScriptEngine *engine;
};

#endif // JSFUNCTIONS_H
