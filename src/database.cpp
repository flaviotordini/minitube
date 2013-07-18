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
#include <QDesktopServices>

static const int DATABASE_VERSION = 1;
static const QString dbName = QLatin1String(Constants::UNIX_NAME) + ".db";
static Database *databaseInstance = 0;

Database::Database() {

    QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QDir().mkpath(dataLocation);
    dbLocation = dataLocation + "/" + dbName;

    QMutexLocker locker(&lock);

    if(QFile::exists(dbLocation)) {
        // check db version
        int databaseVersion = getAttribute("version").toInt();
        if (databaseVersion != DATABASE_VERSION)
            qWarning("Wrong database version: %d", databaseVersion);
    } else createDatabase();
}

Database::~Database() {
    closeConnections();
}

void Database::createDatabase() {

    qWarning() << "Creating the database";

    const QSqlDatabase db = getConnection();

    QSqlQuery("create table subscriptions ("
              "id integer primary key autoincrement,"
              "user_id varchar,"
              "user_name varchar,"
              "name varchar,"
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
              "channel_id integer,"
              "published integer,"
              "added integer,"
              "watched integer,"
              "title varchar,"
              "author varchar,"
              "user_id varchar,"
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

QString Database::getDbLocation() {
    static QString dataLocation = QDesktopServices::storageLocation(
                QDesktopServices::DataLocation);
    return dataLocation + "/" + dbName;
}

bool Database::exists() {
    static bool fileExists = false;
    if (!fileExists)
        fileExists = QFile::exists(getDbLocation());
    return fileExists;
}

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

    const QString threadName = currentThread->objectName();
    // qDebug() << "threadName" << threadName << currentThread;
    if (connections.contains(currentThread)) {
        return connections.value(currentThread);
    } else {
        // qDebug() << "Creating db connection for" << threadName;
        QSqlDatabase connection = QSqlDatabase::addDatabase("QSQLITE", threadName);
        connection.setDatabaseName(dbLocation);
        if(!connection.open()) {
            qWarning() << QString("Cannot connect to database %1 in thread %2").arg(dbLocation, threadName);
        }
        connections.insert(currentThread, connection);
        return connection;
    }
}

QVariant Database::getAttribute(QString name) {
    QSqlQuery query("select value from attributes where name=?", getConnection());
    query.bindValue(0, name);

    bool success = query.exec();
    if (!success) qDebug() << query.lastQuery() << query.boundValues().values() << query.lastError().text();
    if (query.next())
        return query.value(0);
    return QVariant();
}

void Database::setAttribute(QString name, QVariant value) {
    QSqlQuery query(getConnection());
    query.prepare("update attributes set value=? where name=?");
    query.bindValue(0, value);
    query.bindValue(1, name);
    bool success = query.exec();
    if (!success) qDebug() << query.lastError().text();
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
            if (tableName.startsWith("sqlite_") || tableName == "attributes") continue;
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
    foreach(QSqlDatabase connection, connections.values()) {
        // qDebug() << "Closing connection" << connection;
        connection.close();
    }
    connections.clear();
}

void Database::closeConnection() {
    QThread *currentThread = QThread::currentThread();
    if (!connections.contains(currentThread)) return;
    QSqlDatabase connection = connections.take(currentThread);
    // qDebug() << "Closing connection" << connection;
    connection.close();
}

void Database::shutdown() {
    if (!databaseInstance) return;
    QSqlQuery("vacuum", databaseInstance->getConnection());
    databaseInstance->closeConnections();
}
