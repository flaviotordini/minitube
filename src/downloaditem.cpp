#include "downloaditem.h"
#include "networkaccess.h"
#include "video.h"

#include <QDesktopServices>

namespace The {
    NetworkAccess* http();
}

DownloadItem::DownloadItem(Video *video, QUrl url, QString filename, QObject *parent)
    : QObject(parent)
    , m_bytesReceived(0)
    , m_startedSaving(false)
    , m_finishedDownloading(false)
    , m_url(url)
    , m_file(filename)
    , m_reply(0)
    , video(video)
    , m_status(Idle)
{ }

void DownloadItem::start() {
    m_reply = The::http()->simpleGet(m_url);
    init();
}

void DownloadItem::init() {
    if (!m_reply)
        return;

    m_status = Starting;

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
    m_downloadTime.start();

    if (m_reply->error() != QNetworkReply::NoError) {
        error(m_reply->error());
        requestFinished();
    }
}


void DownloadItem::stop() {
    if (m_reply)
        m_reply->abort();
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
    QUrl url = QUrl::fromLocalFile(info.absolutePath());
    QDesktopServices::openUrl(url);
}

void DownloadItem::tryAgain() {
    if (m_reply)
        m_reply->abort();

    if (m_file.exists())
        m_file.remove();

    m_reply = The::http()->simpleGet(m_url);
    init();
    emit statusChanged();
}

void DownloadItem::downloadReadyRead() {

    if (!m_file.isOpen()) {
        if (!m_file.open(QIODevice::WriteOnly)) {
            qDebug() << QString("Error opening output file: %1").arg(m_file.errorString());
            stop();
            emit statusChanged();
            return;
        }
        emit statusChanged();
    }

    m_status = Downloading;
    if (-1 == m_file.write(m_reply->readAll())) {
        /*
        downloadInfoLabel->setText(tr("Error saving: %1")
                                   .arg(m_output.errorString()));
        stopButton->click();
        */
    } else {
        m_startedSaving = true;
        if (m_finishedDownloading)
            requestFinished();
    }
}

void DownloadItem::error(QNetworkReply::NetworkError) {

#ifdef DOWNLOADMANAGER_DEBUG
    qDebug() << "DownloadItem::" << __FUNCTION__ << m_reply->errorString() << m_url;
#endif

    m_errorMessage = m_reply->errorString();
    m_reply = 0;
    m_status = Failed;

    emit finished();
}

QString DownloadItem::errorMessage() const {
    return m_errorMessage;
}

void DownloadItem::metaDataChanged() {
    QVariant locationHeader = m_reply->header(QNetworkRequest::LocationHeader);
    if (locationHeader.isValid()) {
        m_url = locationHeader.toUrl();
        m_reply->deleteLater();
        m_reply = The::http()->simpleGet(m_url);
        init();
        return;
    }

#ifdef DOWNLOADMANAGER_DEBUG
    qDebug() << "DownloadItem::" << __FUNCTION__ << "not handled.";
#endif
}

void DownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    QTime now = QTime::currentTime();
    if (m_lastProgressTime.msecsTo(now) < 200)
        return;

    m_lastProgressTime = now;

    m_bytesReceived = bytesReceived;
    if (bytesTotal > 0) {
        percent = bytesReceived * 100 / bytesTotal;
    }

    emit progress(percent);
    // emit statusChanged();
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

    double timeRemaining = ((double)(bytesTotal() - bytesReceived())) / currentSpeed();

    // When downloading the eta should never be 0
    if (timeRemaining == 0)
        timeRemaining = 1;

    return timeRemaining;
}

double DownloadItem::currentSpeed() const {
    if (m_finishedDownloading)
        return -1.0;

    return m_bytesReceived * 1000.0 / m_downloadTime.elapsed();
}

void DownloadItem::requestFinished() {
    m_reply = 0;
    m_finishedDownloading = true;
    if (!m_startedSaving) {
        return;
    }
    m_file.close();
    m_status = Finished;
    emit statusChanged();
    emit finished();
}

QString DownloadItem::formattedFilesize(qint64 size) {
    /*
    if (size < 1024) return tr("%1 bytes").arg(size);
    else if (size < 1024*1024) return tr("%1 KB").arg(size/1024);
    else if (size < 1024*1024*1024) return tr("%1 MB").arg(size/1024/1024);
    else return tr("%1 GB").arg(size/1024/1024/1024);
    */
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
    /*
    static const int K = 1024;
    if (speed < K) return tr("%1 bytes/s").arg(speed);
    else if (speed < K*K) return tr("%1 KB/s").arg(speed/K);
    else if (speed < K*K*K) return tr("%1 MB/s").arg(speed/K/K);
    else return tr("%1 GB/s").arg(speed/K/K/K);
    */
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

QString DownloadItem::formattedTime(double timeRemaining) {
    QString timeRemainingString = tr("seconds");
    if (timeRemaining > 60) {
        timeRemaining = timeRemaining / 60;
        timeRemainingString = tr("minutes");
    }
    timeRemaining = floor(timeRemaining);
    return tr("%4 %5 remaining")
            .arg(timeRemaining)
            .arg(timeRemainingString);
}
