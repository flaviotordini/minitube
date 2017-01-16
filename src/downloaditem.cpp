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

#include "downloaditem.h"
#include "http.h"
#include "httputils.h"
#include "video.h"

#include <QDesktopServices>
#include <QDebug>

#ifdef APP_MAC
#include "macutils.h"
#endif

DownloadItem::DownloadItem(Video *video, QUrl url, QString filename, QObject *parent)
    : QObject(parent)
    , m_bytesReceived(0)
    , m_startedSaving(false)
    , m_finishedDownloading(false)
    , m_url(url)
    , m_offset(0)
    , sendStatusChanges(true)
    , m_file(filename)
    , m_reply(0)
    , video(video)
    , m_status(Idle)
{
    speedCheckTimer = new QTimer(this);
    speedCheckTimer->setInterval(2000);
    speedCheckTimer->setSingleShot(true);
    connect(speedCheckTimer, SIGNAL(timeout()), SLOT(speedCheck()));

    if (m_file.exists())
        m_file.remove();
}

DownloadItem::~DownloadItem() {
    if (m_reply) {
        delete m_reply;
        m_reply = 0;
    }
    if (video) {
        delete video;
        video = 0;
    }
}

bool DownloadItem::needsDownload(qint64 offset) {
    return offset < m_offset || offset > m_offset + m_bytesReceived;
}

bool DownloadItem::isBuffered(qint64 offset) {
    QMap<qint64, qint64>::iterator i;
    for (i = buffers.begin(); i != buffers.end(); ++i) {
        if (offset >= i.key() && offset <= i.value()) {
            // qDebug() << "Buffered! " << i.key() << ":" << i.value();
            return true;
        }
    }
    // qDebug() << offset << "is not buffered";
    return false;
}

qint64 DownloadItem::blankAtOffset(qint64 offset) {
    // qDebug() << buffers;
    QMap<qint64, qint64>::iterator i;
    for (i = buffers.begin(); i != buffers.end(); ++i) {
        if (offset >= i.key() && offset <= i.value()) {
            // qDebug() << "Offset was" << offset << "now" << i.value();
            return i.value() + 1;
        }
    }
    return offset;
}

void DownloadItem::seekTo(qint64 offset, bool sendStatusChanges) {
    // qDebug() << __PRETTY_FUNCTION__ << offset << sendStatusChanges;
    stop();
    if (m_bytesReceived > 0) {
        bool bufferModified = false;
        QMap<qint64, qint64>::iterator i;
        for (i = buffers.begin(); i != buffers.end(); ++i) {
            if (m_offset - 1 <= i.value()) {
                /*
                qDebug() << "Extending existing buffer "
                         << i.key() << i.value() << "now" << m_offset + m_bytesReceived;
                */
                bufferModified = true;
                i.value() = m_offset + m_bytesReceived;
                break;
            }
        }
        if (!bufferModified)
            buffers.insert(m_offset, m_offset + m_bytesReceived);
    }
    m_offset = offset;
    this->sendStatusChanges = sendStatusChanges;
    bool seekSuccess = m_file.seek(offset);
    if (!seekSuccess) {
        qWarning() << "Cannot seek to offset" << offset << "in file" << m_file.fileName();
        return;
    }
    start();
}

void DownloadItem::start() {
    // qDebug() << "Starting download at" << m_offset;
    HttpRequest req;
    req.url = m_url;
    if (m_offset > 0) req.offset = m_offset;
    m_reply = HttpUtils::yt().networkReply(req);

    init();
}

void DownloadItem::init() {
    if (!m_reply)
        return;

    m_status = Starting;
    m_bytesReceived = 0;
    m_startedSaving = false;
    m_finishedDownloading = false;

    // attach to the m_reply
    m_url = m_reply->url();
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(downloadProgress(qint64, qint64)));
    connect(m_reply, SIGNAL(metaDataChanged()),
            this, SLOT(metaDataChanged()));
    connect(m_reply, SIGNAL(finished()),
            this, SLOT(requestFinished()));

    // start timer for the download estimation
    m_totalTime = 0;
    m_downloadTime.start();
    speedCheckTimer->start();

    if (m_reply->error() != QNetworkReply::NoError) {
        error(m_reply->error());
        requestFinished();
    }
}


void DownloadItem::stop() {
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = 0;
    }
    m_status = Idle;
    emit statusChanged();
}

void DownloadItem::open() {
    QFileInfo info(m_file);
    QUrl url = QUrl::fromLocalFile(info.absoluteFilePath());
    QDesktopServices::openUrl(url);
}

void DownloadItem::openFolder() {
    QFileInfo info(m_file);
#ifdef APP_MAC
    mac::showInFinder(info.absoluteFilePath());
#else
    QUrl url = QUrl::fromLocalFile(info.absolutePath());
    QDesktopServices::openUrl(url);
#endif
}

void DownloadItem::tryAgain() {
    stop();
    start();
}

void DownloadItem::downloadReadyRead() {
    if (!m_reply) return;

    if (!m_file.isOpen()) {
        if (!m_file.open(QIODevice::ReadWrite)) {
            qWarning() << QString("Error opening output file: %1").arg(m_file.errorString());
            stop();
            emit statusChanged();
            return;
        }
        emit statusChanged();
    }

    // qWarning() << __PRETTY_FUNCTION__ << m_file.pos();
    if (-1 == m_file.write(m_reply->readAll())) {
        qWarning() << "Error saving." << m_file.errorString();
    } else {

        m_startedSaving = true;

        // if (m_finishedDownloading) requestFinished();
    }
}

void DownloadItem::error(QNetworkReply::NetworkError) {

    if (m_reply) {
        qWarning() << m_reply->errorString() << m_reply->url().toEncoded();
        m_errorMessage = m_reply->errorString();
    }

    m_reply = 0;
    m_status = Failed;

    emit finished();
}

QString DownloadItem::errorMessage() const {
    return m_errorMessage;
}

void DownloadItem::metaDataChanged() {
    if (!m_reply) return;
    QVariant locationHeader = m_reply->header(QNetworkRequest::LocationHeader);
    if (locationHeader.isValid()) {
        m_url = locationHeader.toUrl();
        // qDebug() << "Redirecting to" << m_url;
        tryAgain();
        return;
    }
    // qDebug() << m_reply->rawHeaderList();
}

int DownloadItem::initialBufferSize() {
    // qDebug() << video->getDefinitionCode();
    switch (video->getDefinitionCode()) {
    case 18:
        return 1024*512;
    case 22:
        return 1024*1024;
    case 37:
        return 1024*1024*2;
    }
    return 1024*128;
}

void DownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {

    // qDebug() << __PRETTY_FUNCTION__ << bytesReceived << bytesTotal << m_downloadTime.elapsed();

    m_bytesReceived = bytesReceived;

    if (!sendStatusChanges) return;

    if (m_lastProgressTime.elapsed() < 150) return;
    m_lastProgressTime.start();

    if (m_status != Downloading) {

        int neededBytes = (int) (bytesTotal * .005);
        int bufferSize = initialBufferSize();
        if (bufferSize > bytesTotal) bufferSize = bytesTotal;
        // qDebug() << bytesReceived << bytesTotal << neededBytes << bufferSize << m_downloadTime.elapsed();
        if (bytesReceived > bufferSize
                && bytesReceived > neededBytes
                && m_downloadTime.elapsed() > 2000) {
            emit bufferProgress(100);
            m_status = Downloading;
            emit statusChanged();
        } else {
            int bytes = qMax(bufferSize, neededBytes);
            int bufferPercent = 0;
            if (bytes > 0)
                bufferPercent = bytesReceived * 100 / bytes;
            emit bufferProgress(bufferPercent);
        }

    } else {

        if (bytesTotal > 0) {
            int percent = bytesReceived * 100 / bytesTotal;
            if (percent != this->percent) {
                this->percent = percent;
                emit progress(percent);
            }
        }

    }
}

void DownloadItem::speedCheck() {
    if (!m_reply) return;
    int bytesTotal = m_reply->size();
    int bufferSize = initialBufferSize();
    if (bufferSize > bytesTotal) bufferSize = 0;
    if (m_bytesReceived < bufferSize / 3) {
        stop();

        // too slow! retry
        qDebug() << "Retrying...";
        connect(video, SIGNAL(gotStreamUrl(QUrl)), SLOT(gotStreamUrl(QUrl)), Qt::UniqueConnection);
        video->loadStreamUrl();
    }
}

void DownloadItem::gotStreamUrl(QUrl /*streamUrl*/) {

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender";
        return;
    }
    video->disconnect(this);

    m_url = video->getStreamUrl();
    start();
}

qint64 DownloadItem::bytesTotal() const {
    if (!m_reply) return 0;
    return m_reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
}

qint64 DownloadItem::bytesReceived() const {
    return m_bytesReceived;
}

double DownloadItem::remainingTime() const {
    if (m_finishedDownloading)
        return -1.0;

    double speed = currentSpeed();
    double timeRemaining = 0.0;
    if (speed > 0.0)
        timeRemaining = ((double)(bytesTotal() - bytesReceived())) / speed;

    // When downloading the eta should never be 0
    if (timeRemaining == 0)
        timeRemaining = 1;

    return timeRemaining;
}

double DownloadItem::currentSpeed() const {
    if (m_finishedDownloading)
        return -1.0;

    int elapsed = m_downloadTime.elapsed();
    double speed = -1.0;
    if (elapsed > 0)
        speed = m_bytesReceived * 1000.0 / elapsed;
    return speed;
}

void DownloadItem::requestFinished() {
    if (!m_startedSaving) {
        qDebug() << "Request finished but never started saving";
        tryAgain();
        return;
    }

    if (m_bytesReceived <= 0) {
        qDebug() << "Request finished but saved 0 bytes";
        tryAgain();
        return;
    }

    m_finishedDownloading = true;

    if (m_status == Starting) {
        m_status = Downloading;
        emit statusChanged();
    }
    if (m_offset == 0) m_file.close();
    m_status = Finished;
    m_totalTime = m_downloadTime.elapsed() / 1000.0;
    emit statusChanged();
    emit finished();
    m_reply->deleteLater();
    m_reply = 0;
}

QString DownloadItem::formattedFilesize(qint64 size) {
    QString unit;
    if (size < 1024) {
        unit = tr("bytes");
    } else if (size < 1024*1024) {
        size /= 1024;
        unit = tr("KB");
    } else {
        size /= 1024*1024;
        unit = tr("MB");
    }
    return QString(QLatin1String("%1 %2")).arg(size).arg(unit);
}

QString DownloadItem::formattedSpeed(double speed) {
    int speedInt = (int) speed;
    QString unit;
    if (speedInt < 1024) {
        unit = tr("bytes/sec");
    } else if (speedInt < 1024*1024) {
        speedInt /= 1024;
        unit = tr("KB/sec");
    } else {
        speedInt /= 1024*1024;
        unit = tr("MB/sec");
    }
    return QString(QLatin1String("%1 %2")).arg(speedInt).arg(unit);
}

QString DownloadItem::formattedTime(double timeRemaining, bool remaining) {
    QString timeRemainingString = tr("seconds");
    if (timeRemaining > 60) {
        timeRemaining = timeRemaining / 60;
        timeRemainingString = tr("minutes");
    }
    timeRemaining = floor(timeRemaining);
    QString msg = remaining ? tr("%4 %5 remaining") : "%4 %5";
    return msg
            .arg(timeRemaining)
            .arg(timeRemainingString);
}
