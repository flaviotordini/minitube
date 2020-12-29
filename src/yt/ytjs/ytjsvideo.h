#ifndef YTJSVIDEO_H
#define YTJSVIDEO_H

#include <QtCore>
class YTJSVideo : public QObject {
    Q_OBJECT
public:
    explicit YTJSVideo(const QString &videoId, QObject *parent = nullptr);
    void loadStreamUrl();
    int getDefinitionCode() const { return definitionCode; }

signals:
    void gotStreamUrl(const QString &videoUrl, const QString &audioUrl);
    void errorStreamUrl(const QString &message);

private:
    QString videoId;
    bool loadingStreamUrl = false;
    int definitionCode;
};

#endif // YTJSVIDEO_H
