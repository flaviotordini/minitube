#ifndef YTCATEGORIES_H
#define YTCATEGORIES_H

#include <QtNetwork>

struct YTCategory {
    QString term;
    QString label;
};

class YTCategories : public QObject {

    Q_OBJECT

public:
    YTCategories(QObject *parent = 0);
    void loadCategories(QString language = QString());
    
signals:
    void categoriesLoaded(const QList<YTCategory> &);
    void error(QString message);

private slots:
    void parseCategories(QByteArray bytes);
    void requestError(QNetworkReply *reply);

private:
    QString lastLanguage;

};

#endif // YTCATEGORIES_H
