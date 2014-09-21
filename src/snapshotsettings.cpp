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

#include "snapshotsettings.h"
#include "mainwindow.h"
#include <QDesktopServices>
#ifdef APP_MAC
#include "macutils.h"
#endif

SnapshotSettings::SnapshotSettings(QWidget *parent) : QWidget(parent) {
    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 10, 0);
    layout->setSpacing(5);

    thumb = new QToolButton();
    thumb->setStyleSheet("border:0");
    thumb->setCursor(Qt::PointingHandCursor);
    connect(thumb, SIGNAL(clicked()), SLOT(openFile()));
    layout->addWidget(thumb);

    message = new QLabel();
    connect(message, SIGNAL(linkActivated(QString)), this, SLOT(showFile()));
    layout->addWidget(message);

    changeFolderButton = new QPushButton();
    changeFolderButton->setAttribute(Qt::WA_MacMiniSize);
    changeFolderButton->setText(tr("Change location..."));
    connect(changeFolderButton, SIGNAL(clicked()), SLOT(changeFolder()));
    layout->addWidget(changeFolderButton);
}

void SnapshotSettings::setSnapshot(const QPixmap &pixmap, const QString &filename) {
    QPixmap p = pixmap.scaledToHeight(message->sizeHint().height());
    QIcon icon(p);
    thumb->setIcon(icon);
    thumb->setIconSize(p.size());
    // thumb->setPixmap(pixmap.scaledToHeight(message->sizeHint().height()));

    this->filename = filename;
    QFileInfo fileInfo(filename);
    QString path = fileInfo.absolutePath();
    QString display = displayPath(path);

    QString msg = tr("Snapshot saved to %1")
                .arg("<a href='showFile' style='text-decoration:none; color:palette(text); font-weight:bold'>%1</a>")
                .arg(display);
    message->setText(msg);
}

void SnapshotSettings::setCurrentLocation(const QString &location) {
    QSettings settings;
    settings.setValue("snapshotsFolder", location);
}

QString SnapshotSettings::getCurrentLocation() {
    QSettings settings;
    QString location = settings.value("snapshotsFolder").toString();
    if (location.isEmpty() || !QFile::exists(location)) {
#if QT_VERSION >= 0x050000
        location = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
#else
        location = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
#endif
    }
    return location;
}

QString SnapshotSettings::displayPath(const QString &path) {
#if QT_VERSION >= 0x050000
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#else
    QString home = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    QString displayPath = path;
    displayPath = displayPath.remove(home + "/");
    return displayPath;
}

void SnapshotSettings::changeFolder() {
    QString path;
#ifdef APP_MAC
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::ReadOnly);
#if QT_VERSION >= 0x050000
    path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#else
    path = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    dialog->setDirectory(path);
    dialog->open(this, SLOT(folderChosen(const QString &)));
#else

#if QT_VERSION >= 0x050000
    path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#else
    path = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    QString folder = QFileDialog::getExistingDirectory(window(), QString(),
                                                       path,
                                                       QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::ReadOnly);
    folderChosen(folder);
#endif
}

void SnapshotSettings::folderChosen(const QString &dir) {
    if (!dir.isEmpty()) {
        setCurrentLocation(dir);
        QString status = tr("Snapshots location changed.");
        MainWindow::instance()->showMessage(status);
    }
}

void SnapshotSettings::showFile() {
    QFileInfo info(filename);
#ifdef APP_MAC
    mac::showInFinder(info.absoluteFilePath());
#else
    QUrl url = QUrl::fromLocalFile(info.absolutePath());
    QDesktopServices::openUrl(url);
#endif
}

void SnapshotSettings::openFile() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
}
