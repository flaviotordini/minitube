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

#include "database.h"
#include "constants.h"
#include <QtDebug>

static const int DATABASE_VERSION = 1;
static const QString dbName = QLatin1String(Constants::UNIX_NAME) + ".db";
static Database *databaseInstance = 0;

Database::Database() {
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (!QDir().mkpath(dataLocation)) {
        qCritical() << "Failed to create directory " << dataLocation;
    }
    dbLocation = dataLocation + "/" + dbName;
    qDebug() << "Creating database instance" << dbLocation;

    QMutexLocker locker(&lock);

    if (QFile::exists(dbLocation)) {
        // check db version
        int databaseVersion = getAttribute("version").toInt();
        if (databaseVersion > DATABASE_VERSION)
            qWarning("Wrong database version: %d", databaseVersion);

        if (!getAttribute("channelIdFix").toBool())
            fixChannelIds();

    } else createDatabase();
}

Database::~Database() {
    qDebug() << "Destroying database instance" << this;
    closeConnections();
}

void Database::createDatabase() {
#ifdef APP_LINUX
    // Qt5 changed its "data" path. Try to move the old db to the new path
    QString homeLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString qt4DataLocation = homeLocation + "/.local/share/data/" + Constants::ORG_NAME + "/" + Constants::NAME;
    QString oldDbLocation = qt4DataLocation + "/" + dbName;
    qDebug() << oldDbLocation;
    if (QFile::exists(oldDbLocation)) {
        if (QFile::copy(oldDbLocation, dbLocation)) {
            qDebug() << "Moved db from" << oldDbLocation << "to" << dbLocation;
            return;
        }
    }
#endif

    qDebug() << "Creating the database";

    const QSqlDatabase db = getConnection();

    QSqlQuery("create table subscriptions ("
              "id integer primary key autoincrement,"
              "user_id varchar," // this is really channel_id
              "user_name varchar," // obsolete yt2 username
              "name varchar," // this is really channel_title
              "description varchar,"
              "thumb_url varchar,"
              "country varchar,"
              "added integer,"
              "checked integer," // last check for videos on YT APIs
              "updated integer," // most recent video added
              "watched integer," // last time the user watched this channel
              "loaded integer," // last time channel metadata was loaded from YT APIs
              "notify_count integer," // new videos since "watched"
              "views integer)" // number of times the user watched this channel
              , db);
    QSqlQuery("create unique index idx_user_id on subscriptions(user_id)", db);

    QSqlQuery("create table subscriptions_videos ("
              "id integer primary key autoincrement,"
              "video_id varchar,"
              "channel_id integer," // this is really subscription_id
              "published integer,"
              "added integer,"
              "watched integer,"
              "title varchar,"
              "author varchar," // this is really channel_title
              "user_id varchar," // this is really channel_id
              "description varchar,"
              "url varchar,"
              "thumb_url varchar,"
              "views integer,"
              "duration integer)"
              , db);
    QSqlQuery("create unique index idx_video_id on subscriptions_videos(video_id)", db);

    QSqlQuery("create table attributes (name varchar, value)", db);
    QSqlQuery("insert into attributes (name, value) values ('version', "
              + QString::number(DATABASE_VERSION) + ")", db);
}

// static
QString Database::getDbLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + dbName;
}

// static
bool Database::exists() {
    static bool fileExists = false;
    if (!fileExists) {
        fileExists = QFile::exists(getDbLocation());
#ifdef APP_LINUX
        if (!fileExists) {
            QString homeLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            QString qt4DataLocation = homeLocation + "/.local/share/data/" + Constants::ORG_NAME + "/" + Constants::NAME;
            QString oldDbLocation = qt4DataLocation + "/" + dbName;
            fileExists = QFile::exists(oldDbLocation);
        }
#endif
    }
    return fileExists;
}

// static
Database& Database::instance() {
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if (!databaseInstance) databaseInstance = new Database();
    return *databaseInstance;
}

QSqlDatabase Database::getConnection() {
    QThread *currentThread = QThread::currentThread();
    if (!currentThread) {
        qDebug() << "current thread is null";
        return QSqlDatabase();
    }

    const uintptr_t threadId = (std::uintptr_t)(currentThread);
    if (connections.contains(threadId)) {
        return connections.value(threadId);
    } else {
        qDebug() << "Creating db connection for" << threadId;
        QSqlDatabase connection = QSqlDatabase::addDatabase("QSQLITE", QString::number(threadId));
        connection.setDatabaseName(dbLocation);
        if(!connection.open()) {
            qWarning() << QString("Cannot connect to database %1 in thread %2")
                                  .arg(dbLocation, threadId);
        }
        connections.insert(threadId, connection);
        return connection;
    }
}

QVariant Database::getAttribute(const QString &name) {
    QSqlQuery query("select value from attributes where name=?", getConnection());
    query.bindValue(0, name);

    bool success = query.exec();
    if (!success) qDebug() << query.lastQuery() << query.boundValues() << query.lastError().text();
    if (query.next())
        return query.value(0);
    return QVariant();
}

void Database::setAttribute(const QString &name, const QVariant &value) {
    QSqlQuery query(getConnection());
    query.prepare("insert or replace into attributes (name, value) values (?,?)");
    query.bindValue(0, name);
    query.bindValue(1, value);
    bool success = query.exec();
    if (!success) qWarning() << query.lastError().text();
}

void Database::fixChannelIds() {
    if (!getConnection().transaction())
        qWarning() << "Transaction failed" << __PRETTY_FUNCTION__;

    qWarning() << "Fixing channel ids";

    QSqlQuery query(getConnection());
    bool success = query.exec("update subscriptions set user_id='UC' || user_id where user_id not like 'UC%'");
    if (!success) qWarning() << query.lastError().text();

    query = QSqlQuery(getConnection());
    success = query.exec("update subscriptions_videos set user_id='UC' || user_id where user_id not like 'UC%'");
    if (!success) qWarning() << query.lastError().text();

    setAttribute("channelIdFix", 1);

    if (!getConnection().commit())
        qWarning() << "Commit failed" << __PRETTY_FUNCTION__;
}

/**
  * After calling this method you have to reacquire a valid instance using instance()
  */
void Database::drop() {
    /// closeConnections();
    if (!QFile::remove(dbLocation)) {
        qWarning() << "Cannot delete database" << dbLocation;

        // fallback to delete records in tables
        const QSqlDatabase db = getConnection();
        QSqlQuery query(db);
        if (!query.exec("select name from sqlite_master where type='table'")) {
            qWarning() << query.lastQuery() << query.lastError().text();
        }

        while (query.next()) {
            QString tableName = query.value(0).toString();
            if (tableName.startsWith("sqlite_") || tableName == QLatin1String("attributes")) continue;
            QString dropSQL = "delete from " + tableName;
            QSqlQuery query2(db);
            if (!query2.exec(dropSQL))
                qWarning() << query2.lastQuery() << query2.lastError().text();
        }

        query.exec("delete from sqlite_sequence");

    }
    if (databaseInstance) delete databaseInstance;
    databaseInstance = 0;
}

void Database::closeConnections() {
    qDebug() << "Closing all connections";
    for (QSqlDatabase connection : qAsConst(connections)) {
        qDebug() << "Closing connection" << connection;
        connection.close();
    }
    connections.clear();
}

void Database::closeConnection() {
    QThread *currentThread = QThread::currentThread();
    const uintptr_t threadId = (std::uintptr_t)(currentThread);
    if (!connections.contains(threadId)) return;
    QSqlDatabase connection = connections.take(threadId);
    qDebug() << "Closing connection" << connection;
    connection.close();
}

void Database::shutdown() {
    qDebug() << "Shutting down";
    if (!databaseInstance) return;
    QSqlQuery("vacuum", databaseInstance->getConnection());
    databaseInstance->closeConnections();
}
