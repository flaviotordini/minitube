#include "temporary.h"
#include "constants.h"

static QList<QString> paths;
#ifdef Q_WS_X11
static QString userName;
#endif

Temporary::Temporary() { }

QString Temporary::filename() {

    static const QString tempDir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);

    QString tempFile = tempDir + "/" + Constants::UNIX_NAME + "-" + QString::number(qrand());

#ifdef Q_WS_X11
    if (userName.isNull()) {
        userName = QString(getenv("USERNAME"));
        if (userName.isEmpty())
            userName = QString(getenv("USER"));
    }
    if (!userName.isEmpty())
        tempFile += "-" + userName;
#endif

    if (QFile::exists(tempFile) && !QFile::remove(tempFile)) {
        qDebug() << "Cannot remove temp file" << tempFile;
    }

    paths << tempFile;

    if (paths.size() > 1) {
        QString removedFile = paths.takeFirst();
        if (QFile::exists(removedFile) && !QFile::remove(removedFile)) {
            qDebug() << "Cannot remove temp file" << removedFile;
        }
    }

    return tempFile;

}

void Temporary::deleteAll() {
    foreach(QString path, paths) {
        if (QFile::exists(path) && !QFile::remove(path)) {
            qDebug() << "Cannot remove temp file" << path;
        }
    }
}
