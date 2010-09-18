#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtGui>

class DownloadItem;
class DownloadModel;
class Video;

class DownloadManager : public QObject {

    Q_OBJECT

public:
    static DownloadManager* instance();
    void clear();
    void addItem(Video *video);
    const QList<DownloadItem*> getItems() { return items; }
    DownloadModel* getModel() { return downloadModel; }
    DownloadItem* itemForVideo(Video *video);
    int activeItems();
    QString defaultDownloadFolder();
    QString currentDownloadFolder();

signals:
    void finished();
    void statusMessageChanged(QString status);

private slots:
    void itemFinished();
    void updateStatusMessage();
    void gotStreamUrl(QUrl url);

private:
    DownloadManager(QObject *parent = 0);

    QList<DownloadItem*> items;
    DownloadModel *downloadModel;

};

#endif // DOWNLOADMANAGER_H
