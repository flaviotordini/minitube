#ifndef YTVIDEO_H
#define YTVIDEO_H

#include <QtCore>

class VideoDefinition;

class YTVideo : public QObject {
    Q_OBJECT

public:
    YTVideo(const QString &videoId, QObject *parent);
    void loadStreamUrl();
    int getDefinitionCode() const { return definitionCode; }

signals:
    void gotStreamUrl(const QUrl &streamUrl);
    void errorStreamUrl(const QString &message);

private slots:
    void gotVideoInfo(const QByteArray &bytes);
    void errorVideoInfo(const QString &message);
    void scrapeWebPage(const QByteArray &bytes);
    void parseJsPlayer(const QByteArray &bytes);
    void parseDashManifest(const QByteArray &bytes);

private:
    void getVideoInfo();
    void parseFmtUrlMap(const QString &fmtUrlMap, bool fromWebPage = false);
    void captureFunction(const QString &name, const QString &js);
    void captureObject(const QString &name, const QString &js);
    QString decryptSignature(const QString &s);
    void saveDefinitionForUrl(const QString &url, const VideoDefinition &definition);

    QString videoId;
    QUrl m_streamUrl;
    int definitionCode;
    bool loadingStreamUrl;
    // current index for the elTypes list
    // needed to iterate on elTypes
    int elIndex;
    bool ageGate;
    QString videoToken;
    QString fmtUrlMap;
    QString sigFuncName;
    QHash<QString, QString> sigFunctions;
    QHash<QString, QString> sigObjects;
    QString dashManifestUrl;
    QString jsPlayer;
};

#endif // YTVIDEO_H
