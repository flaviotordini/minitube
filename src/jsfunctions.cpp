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

#include "jsfunctions.h"
#include "networkaccess.h"
#include <QDesktopServices>
#include "constants.h"

namespace The {
NetworkAccess* http();
}

JsFunctions* JsFunctions::instance() {
    static JsFunctions *i = new JsFunctions();
    return i;
}

JsFunctions::JsFunctions(QObject *parent) : QObject(parent), engine(0) {
    QFile file(jsPath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            parseJs(QString::fromUtf8(file.readAll()));
        else
            qWarning() << file.errorString() << file.fileName();
        QFileInfo info(file);
        if (info.lastModified().toTime_t() < QDateTime::currentDateTime().toTime_t() - 3600)
            loadJs();
    } else {
        QFile resFile(QLatin1String(":/") + jsFilename());
        resFile.open(QIODevice::ReadOnly | QIODevice::Text);
        parseJs(QString::fromUtf8(resFile.readAll()));
        loadJs();
    }
}

void JsFunctions::parseJs(const QString &js) {
    if (js.isEmpty()) return;
    if (engine) delete engine;
    engine = new QScriptEngine();
    engine->evaluate(js);
}

const QLatin1String & JsFunctions::jsFilename() {
    static const QLatin1String filename("functions.js");
    return filename;
}

const QString & JsFunctions::jsPath() {
    static const QString path(
                QDesktopServices::storageLocation(QDesktopServices::DataLocation)
                + "/" + jsFilename());
    return path;
}

void JsFunctions::loadJs() {
    QUrl url(QLatin1String(Constants::WEBSITE) + "-ws/" + jsFilename());
    NetworkReply* reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotJs(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorJs(QNetworkReply*)));
}

void JsFunctions::gotJs(QByteArray bytes) {
    parseJs(QString::fromUtf8(bytes));
    QFile file(jsPath());
    if (!file.open(QIODevice::WriteOnly))
        qWarning() << file.errorString() << file.fileName();
    QDataStream stream(&file);
    stream.writeRawData(bytes.constData(), bytes.size());
}

void JsFunctions::errorJs(QNetworkReply *reply) {
    qWarning() << "Cannot get" << jsFilename() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                  << reply->url().toString() << reply->errorString();
}

QString JsFunctions::evaluateFunction(const QString &function) {
    if (!engine) return QString();
    QScriptValue value = engine->evaluate(function);
    if (value.isUndefined())
        qWarning() << "Undefined result for" << function;
    if (value.isError())
        qWarning() << "Error in" << function << value.toString();

    return value.toString();
}

QString JsFunctions::decryptSignature(const QString &s) {
    return evaluateFunction("decryptSignature('" + s + "')");
}

QString JsFunctions::decryptAgeSignature(const QString &s) {
    return evaluateFunction("decryptAgeSignature('" + s + "')");
}
