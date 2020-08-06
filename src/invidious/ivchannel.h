#ifndef IVCHANNEL_H
#define IVCHANNEL_H

#include <QtCore>

class IVChannel : public QObject {
    Q_OBJECT

public:
    IVChannel(const QString &id, QObject *parent = nullptr);

    QString getDisplayName() const { return displayName; }
    QString getDescription() const { return description; }
    QString getThumbnailUrl() const { return thumbnailUrl; }

signals:
    void loaded();
    void error(QString message);

private:
    QString displayName;
    QString description;
    QString thumbnailUrl;
};

#endif // IVCHANNEL_H
