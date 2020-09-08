#ifndef YTJSCHANNEL_H
#define YTJSCHANNEL_H

#include <QtCore>

class YTJSChannel : public QObject {
    Q_OBJECT

public:
    YTJSChannel(const QString &id, QObject *parent = nullptr);

    QString getDisplayName() const { return displayName; }
    QString getDescription() const { return description; }
    QString getThumbnailUrl() const { return thumbnailUrl; }

signals:
    void loaded();
    void error(QString message);

private:
    void load(const QString &channelId);

    QString displayName;
    QString description;
    QString thumbnailUrl;
};

#endif // YTJSCHANNEL_H
