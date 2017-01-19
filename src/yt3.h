#ifndef YT3_H
#define YT3_H

#include <QtCore>

class HttpReply;

class YT3 : public QObject {

    Q_OBJECT

public:
    static YT3 &instance();
    static const QString &baseUrl();

    void testApiKey();
    void addApiKey(QUrl &url);
    QUrl method(const QString &name);

signals:
    void gotChannelId(QString channelId);

private slots:
    void testResponse(const HttpReply &reply);

private:
    YT3();

    QStringList keys;
    QString key;
};

#endif // YT3_H
