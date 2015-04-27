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
