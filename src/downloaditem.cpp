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
{
    speedCheckTimer = new QTimer(this);
    speedCheckTimer->setInterval(2000);
    speedCheckTimer->setSingleShot(true);
    connect(speedCheckTimer, SIGNAL(timeout()), SLOT(speedCheck()));
}

DownloadItem::~DownloadItem() {
    if (m_reply) delete m_reply;
    if (video) delete video;
}

void DownloadItem::start() {
    m_reply = The::http()->simpleGet(m_url);
    init();
}

void DownloadItem::init() {
    if (!m_reply)
        return;

    if (m_file.exists())
        m_file.remove();

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
    speedCheckTimer->start();

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
        if (!m_file.open(QIODevice::ReadWrite)) {
            qDebug() << QString("Error opening output file: %1").arg(m_file.errorString());
            stop();
            emit statusChanged();
            return;
        }
        emit statusChanged();
    }

    if (-1 == m_file.write(m_reply->readAll())) {
        /*
        downloadInfoLabel->setText(tr("Error saving: %1")
                                   .arg(m_output.errorString()));
        stopButton->click();
        */
    } else {
        m_startedSaving = true;
        if (m_status != Downloading) {
            // m_status = Downloading;
            // emit statusChanged();
        } else if (m_finishedDownloading)
            requestFinished();
    }
}

void DownloadItem::error(QNetworkReply::NetworkError) {

#ifdef DOWNLOADMANAGER_DEBUG
    qDebug() << "DownloadItem::" << __FUNCTION__ << m_reply->errorString() << m_url;
#endif

    if (m_reply) {
        qDebug() << m_reply->errorString();
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
    QVariant locationHeader = m_reply->header(QNetworkRequest::LocationHeader);
    if (locationHeader.isValid()) {
        m_url = locationHeader.toUrl();
        // qDebug() << "Redirecting to" << m_url;
        m_reply->deleteLater();
        m_reply = The::http()->simpleGet(m_url);
        init();
        return;
    }

#ifdef DOWNLOADMANAGER_DEBUG
    qDebug() << "DownloadItem::" << __FUNCTION__ << "not handled.";
#endif
}

int DownloadItem::initialBufferSize() {
    // qDebug() << video->getDefinitionCode();
    switch (video->getDefinitionCode()) {
    case 18:
        return 1024*192;
    case 22:
        return 1024*512;
    case 37:
        return 1024*768;
    }
    return 1024*128;
}

void DownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {

    if (m_lastProgressTime.elapsed() < 150) return;
    m_lastProgressTime.start();

    m_bytesReceived = bytesReceived;

    if (m_status != Downloading) {

        int neededBytes = (int) (bytesTotal * .001);
        // qDebug() << bytesReceived << bytesTotal << neededBytes << m_downloadTime.elapsed();
        int bufferSize = initialBufferSize();
        if (bytesReceived > bufferSize
            && bytesReceived > neededBytes
            && m_downloadTime.elapsed() > 1000 ) {
            emit bufferProgress(100);
            m_status = Downloading;
            emit statusChanged();
        } else {
            int bufferPercent = bytesReceived * 100 / qMax(bufferSize, neededBytes);
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
    if (m_bytesReceived < initialBufferSize() / 3) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = 0;

        // too slow! retry
        qDebug() << "Retrying...";
        connect(video, SIGNAL(gotStreamUrl(QUrl)), SLOT(gotStreamUrl(QUrl)));
        video->loadStreamUrl();
    }
}

void DownloadItem::gotStreamUrl(QUrl streamUrl) {

    Video *video = static_cast<Video *>(sender());
    if (!video) {
        qDebug() << "Cannot get sender";
        return;
    }
    video->disconnect(this);

    m_reply = The::http()->simpleGet(video->getStreamUrl());
    init();
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
        qDebug() << "Request finished but never started saving";
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
