#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <QtCore>

class Video;

class VideoSource : public QObject {

    Q_OBJECT

public:
    VideoSource(QObject *parent = 0) : QObject(parent) { }
    virtual void loadVideos(int max, int skip) = 0;
    virtual void abort() = 0;
    virtual const QStringList & getSuggestions() = 0;
    virtual QString getName() = 0;

public slots:
    void setParam(QString name, QVariant value);

signals:
    void gotVideo(Video *video);
    void finished(int total);
    void error(QString message);
    void nameChanged(QString name);

};

#endif // VIDEOSOURCE_H
