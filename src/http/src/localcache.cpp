#include "localcache.h"

LocalCache *LocalCache::instance(const QString &name) {
    static QHash<QString, LocalCache*> instances;
    QHash<QString, LocalCache*>::const_iterator i = instances.constFind(name);
    if (i != instances.constEnd()) return i.value();
    LocalCache *instance = new LocalCache(name);
    instances.insert(name, instance);
    return instance;
}

LocalCache::LocalCache(const QString &name) : name(name),
    maxSeconds(86400*30),
    maxSize(1024*1024*100),
    size(0),
    expiring(false),
    insertCount(0) {
    directory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') +
            name + QLatin1Char('/');
#ifndef QT_NO_DEBUG_OUTPUT
    hits = 0;
    misses = 0;
#endif
}

LocalCache::~LocalCache() {
#ifndef QT_NO_DEBUG_OUTPUT
    debugStats();
#endif
}

QString LocalCache::hash(const QString &s) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(s.toUtf8());
    QString h = QString::number(*(qlonglong*)hash.result().constData(), 36);
    return h.at(0) + QLatin1Char('/') + h.at(1) + QLatin1Char('/') + h.mid(2);
}

bool LocalCache::isCached(const QString &key) {
    QString path = cachePath(key);
    bool cached = (QFile::exists(path) &&
                   (maxSeconds == 0 ||
                    QDateTime::currentDateTime().toTime_t() - QFileInfo(path).created().toTime_t() < maxSeconds));
#ifndef QT_NO_DEBUG_OUTPUT
    if (!cached) misses++;
#endif
    return cached;
}

QByteArray LocalCache::value(const QString &key) {
    QString path = cachePath(key);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << __PRETTY_FUNCTION__ << file.fileName() << file.errorString();
#ifndef QT_NO_DEBUG_OUTPUT
        misses++;
#endif
        return QByteArray();
    }
#ifndef QT_NO_DEBUG_OUTPUT
    hits++;
#endif
    return file.readAll();
}

void LocalCache::insert(const QString &key, const QByteArray &value) {
    QString path = cachePath(key);
    QFileInfo info(path);
    if (!info.exists())
        QDir().mkpath(info.absolutePath());
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(value);
    file.close();

    // expire cache every n inserts
    if (maxSize > 0 && ++insertCount % 100 == 0) {
        if (size == 0) size = expire();
        else {
            size += value.size();
            if (size > maxSize) size = expire();
        }
    }
}

bool LocalCache::clear() {
#ifndef QT_NO_DEBUG_OUTPUT
    hits = 0;
    misses = 0;
#endif
    size = 0;
    insertCount = 0;
    return QDir(directory).removeRecursively();
}

QString LocalCache::cachePath(const QString &key) const {
    return directory + key;
}

qint64 LocalCache::expire() {
    if (expiring) return size;
    expiring = true;

    QDir::Filters filters = QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot;
    QDirIterator it(directory, filters, QDirIterator::Subdirectories);

    QMultiMap<QDateTime, QString> cacheItems;
    qint64 totalSize = 0;
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo info = it.fileInfo();
        cacheItems.insert(info.created(), path);
        totalSize += info.size();
        qApp->processEvents();
    }

    int removedFiles = 0;
    qint64 goal = (maxSize * 9) / 10;
    QMultiMap<QDateTime, QString>::const_iterator i = cacheItems.constBegin();
    while (i != cacheItems.constEnd()) {
        if (totalSize < goal)
            break;
        QString name = i.value();
        QFile file(name);
        qint64 size = file.size();
        file.remove();
        totalSize -= size;
        ++removedFiles;
        ++i;
        qApp->processEvents();
    }
#ifndef QT_NO_DEBUG_OUTPUT
    debugStats();
    if (removedFiles > 0) {
        qDebug() << "Removed:" << removedFiles
                 << "Kept:" << cacheItems.count() - removedFiles
                 << "New Size:" << totalSize;
    }
#endif

    expiring = false;

    return totalSize;
}

#ifndef QT_NO_DEBUG_OUTPUT
void LocalCache::debugStats() {
    int total = hits + misses;
    if (total > 0) {
        qDebug() << "Cache:" << name << '\n'
                 << "Inserts:" << insertCount << '\n'
                 << "Requests:" << total << '\n'
                 << "Hits:" << hits << (hits*100)/total  << "%\n"
                 << "Misses:" << misses << (misses*100)/total << "%";
    }
}
#endif
