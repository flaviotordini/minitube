#ifndef LOCALCACHE_H
#define LOCALCACHE_H

#include <QtCore>

/**
 * @brief Not thread-safe
 */
class LocalCache {
public:
    static LocalCache *instance(const QString &name);
    ~LocalCache();
    static QString hash(const QString &s);

    void setMaxSeconds(uint value) { maxSeconds = value; }
    void setMaxSize(uint value) { maxSize = value; }
    bool isCached(const QString &key);
    QByteArray value(const QString &key);
    void insert(const QString &key, const QByteArray &value);
    bool clear();

private:
    LocalCache(const QString &name);
    QString cachePath(const QString &key) const;
    qint64 expire();
#ifndef QT_NO_DEBUG_OUTPUT
    void debugStats();
#endif

    QString name;
    QString directory;
    uint maxSeconds;
    qint64 maxSize;
    qint64 size;
    bool expiring;
    uint insertCount;

#ifndef QT_NO_DEBUG_OUTPUT
    uint hits;
    uint misses;
#endif
};

#endif // LOCALCACHE_H
