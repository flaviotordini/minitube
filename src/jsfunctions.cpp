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
#include "compatibility/qurlqueryhelper.h"

namespace The {
NetworkAccess* http();
}

JsFunctions* JsFunctions::instance() {
    static JsFunctions *i = new JsFunctions(QLatin1String(Constants::WEBSITE) + "-ws/functions.js");
    return i;
}

JsFunctions::JsFunctions(const QString &url, QObject *parent) : QObject(parent), url(url), engine(0) {
    QFile file(jsPath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            parseJs(QString::fromUtf8(file.readAll()));
        else
            qWarning() << "Cannot open" << file.errorString() << file.fileName();
        QFileInfo info(file);
        bool stale = info.size() == 0 || info.lastModified().toTime_t() < QDateTime::currentDateTime().toTime_t() - 1800;
        if (stale) loadJs();
    } else {
        QFile resFile(QLatin1String(":/") + jsFilename());
        resFile.open(QIODevice::ReadOnly | QIODevice::Text);
        parseJs(QString::fromUtf8(resFile.readAll()));
        loadJs();
    }
}

void JsFunctions::parseJs(const QString &js) {
    if (js.isEmpty()) return;
    // qDebug() << "Parsing" << js;
    if (engine) delete engine;
    engine = new QScriptEngine(this);
    engine->evaluate(js);
    emit ready();
}

QString JsFunctions::jsFilename() {
    return QFileInfo(url).fileName();
}

QString JsFunctions::jsPath() {
    return QString(
            #if QT_VERSION >= 0x050000
                QStandardPaths::writableLocation(QStandardPaths::DataLocation)
            #else
                QDesktopServices::storageLocation(QDesktopServices::DataLocation)
            #endif
                + "/" + jsFilename());
}

void JsFunctions::loadJs() {
    QUrl url(this->url);
    {
        QUrlQueryHelper urlHelper(url);
        urlHelper.addQueryItem("v", Constants::VERSION);
    }

    NetworkReply* reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(gotJs(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), SLOT(errorJs(QNetworkReply*)));
}

void JsFunctions::gotJs(const QByteArray &bytes) {
    if (bytes.isEmpty()) {
        qWarning() << "Got empty js";
        return;
    }
    QFile file(jsPath());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write" << file.errorString() << file.fileName();
        return;
    }
    QDataStream stream(&file);
    stream.writeRawData(bytes.constData(), bytes.size());
    parseJs(QString::fromUtf8(bytes));
}

void JsFunctions::errorJs(QNetworkReply *reply) {
    qWarning() << "Cannot get" << jsFilename() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
               << reply->url().toString() << reply->errorString();
}

QScriptValue JsFunctions::evaluate(const QString &js) {
    if (!engine) return QString();
    QScriptValue value = engine->evaluate(js);
    if (value.isUndefined())
        qWarning() << "Undefined result for" << js;
    if (value.isError())
        qWarning() << "Error in" << js << value.toString();

    return value;
}

QString JsFunctions::string(const QString &js) {
    return evaluate(js).toString();
}

QStringList JsFunctions::stringArray(const QString &js) {
    QStringList items;
    QScriptValue array = evaluate(js);
    if (!array.isArray()) return items;
    QScriptValueIterator it(array);
    while (it.hasNext()) {
        it.next();
        QScriptValue value = it.value();
        if (!value.isString()) continue;
        items << value.toString();
    }
    return items;
}

QString JsFunctions::decryptSignature(const QString &s) {
    return string("decryptSignature('" + s + "')");
}

QString JsFunctions::decryptAgeSignature(const QString &s) {
    return string("decryptAgeSignature('" + s + "')");
}

QString JsFunctions::videoIdRE() {
    return string("videoIdRE()");
}

QString JsFunctions::videoTokenRE() {
    return string("videoTokenRE()");
}

QString JsFunctions::videoInfoFmtMapRE() {
    return string("videoInfoFmtMapRE()");
}

QString JsFunctions::webPageFmtMapRE() {
    return string("webPageFmtMapRE()");
}

QString JsFunctions::ageGateRE() {
    return string("ageGateRE()");
}

QString JsFunctions::jsPlayerRE() {
    return string("jsPlayerRE()");
}

QString JsFunctions::signatureFunctionNameRE() {
    return string("signatureFunctionNameRE()");
}

QStringList JsFunctions::apiKeys() {
    return stringArray("apiKeys()");
}
