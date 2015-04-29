/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2015, Flavio Tordini <flavio.tordini@gmail.com>

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

#ifndef QURLQUERYHELPER_H
#define QURLQUERYHELPER_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QUrl)

#if QT_VERSION >= 0x050000
#include <QUrlQuery>

class QUrlQueryHelper {
public:
    QUrlQueryHelper(QUrl &url) : m_url(url), m_urlQuery(url) {}

    bool hasQueryItem(const QString &itemKey) const {
        return m_urlQuery.hasQueryItem(itemKey);
    }

    void addQueryItem(const QString &key, const QString &value) {
        m_urlQuery.addQueryItem(key, value);
    }

    void removeQueryItem(const QString &key) {
        m_urlQuery.removeQueryItem(key);
    }

    ~QUrlQueryHelper() {
        m_url.setQuery(m_urlQuery);
    }

private:
    QUrlQueryHelper(const QUrlQueryHelper&);
    QUrlQueryHelper& operator=(const QUrlQueryHelper&);

    QUrl &m_url;
    QUrlQuery m_urlQuery;
};
#else
typedef QUrl& QUrlQueryHelper;
#endif  // QT_VERSION >= 0x050000

#endif // QURLQUERYHELPER_H
