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

#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QtCore>
#include <QNetworkReply>

class Video;

enum DownloadItemStatus {
    Idle = 0,
    Starting,
    Downloading,
    Finished,
    Failed
};

class DownloadItem : public QObject {

    Q_OBJECT

signals:
    void statusChanged();
    void bufferProgress(int percent);
    void progress(int percent);
    void finished();
    void error(QString);

public:
    DownloadItem(Video *video, QUrl url, QString filename, QObject *parent = 0);
    ~DownloadItem();
    qint64 bytesTotal() const;
    qint64 bytesReceived() const;
    double remainingTime() const;
    double totalTime() { return m_totalTime; }
    double currentSpeed() const;
    int currentPercent() const { return percent; }
    Video* getVideo() const { return video; }
    QString currentFilename() const { return m_file.fileName(); }
    DownloadItemStatus status() const { return m_status; }
    static QString formattedFilesize(qint64 size);
    static QString formattedSpeed(double speed);
    static QString formattedTime(double time, bool remaining = true);
    QString errorMessage() const;

public slots:
    void start();
    void stop();
    void tryAgain();
    void open();
    void openFolder();

private slots:
    void downloadReadyRead();
    void error(QNetworkReply::NetworkError code);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void metaDataChanged();
    void requestFinished();
    void gotStreamUrl(QUrl streamUrl);
    void speedCheck();

private:
    void init();
    int initialBufferSize();

    qint64 m_bytesReceived;
    QTime m_downloadTime;
    bool m_startedSaving;
    bool m_finishedDownloading;
    QTime m_lastProgressTime;
    int percent;
    double m_totalTime;

    QUrl m_url;

    QFile m_file;
    QNetworkReply *m_reply;
    Video *video;

    DownloadItemStatus m_status;
    QString m_errorMessage;

    QTimer *speedCheckTimer;

};

// This is required in order to use QPointer<DownloadItem> as a QVariant
// as used by the Model/View playlist
typedef QPointer<DownloadItem> DownloadItemPointer;
Q_DECLARE_METATYPE(DownloadItemPointer)

#endif // DOWNLOADITEM_H
