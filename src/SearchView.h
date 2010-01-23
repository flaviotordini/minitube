#ifndef __SEARCHVIEW_H__
#define __SEARCHVIEW_H__

#include <QtGui>
#include "View.h"
#include "searchlineedit.h"
#include "updatechecker.h"

class SearchView : public QWidget, public View {

    Q_OBJECT

public:
    SearchView(QWidget *parent);

    void appear() {
        updateRecentKeywords();
        queryEdit->clear();
        queryEdit->setFocus(Qt::OtherFocusReason);
        queryEdit->enableSuggest();
    }

    void disappear() {}

    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        metadata.insert("title", "");
        metadata.insert("description", tr("Make yourself comfortable"));
        return metadata;
    }

public slots:
    void watch(QString query);
    void gotNewVersion(QString version);

signals:
    void search(QString query);

private slots:
    void watch();
    void textChanged(const QString &text);
    void clearRecentKeywords();

private:
    void updateRecentKeywords();
    void checkForUpdate();

    SearchLineEdit *queryEdit;
    QLabel *recentKeywordsLabel;
    QVBoxLayout *recentKeywordsLayout;
    QLabel *message;
    QPushButton *watchButton;

    UpdateChecker *updateChecker;

};

#endif // __SEARCHVIEW_H__
