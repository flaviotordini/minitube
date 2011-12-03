#ifndef __SEARCHVIEW_H__
#define __SEARCHVIEW_H__

#include <QtGui>
#include "View.h"

class SearchLineEdit;
class SearchParams;
class YouTubeSuggest;
class ChannelSuggest;

class SearchView : public QWidget, public View {

    Q_OBJECT

public:
    SearchView(QWidget *parent);
    void updateRecentKeywords();
    void updateRecentChannels();
    void appear();
    void disappear() {}

    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        metadata.insert("title", "");
        metadata.insert("description", tr("Make yourself comfortable"));
        return metadata;
    }

public slots:
    void watch(QString query);
    void watchChannel(QString channel);
    void watchKeywords(QString query);

signals:
    void search(SearchParams*);

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void watch();
    void textChanged(const QString &text);
    void searchTypeChanged(int index);

private:
    YouTubeSuggest *youtubeSuggest;
    ChannelSuggest *channelSuggest;

    QComboBox *typeCombo;
    SearchLineEdit *queryEdit;
    QLabel *recentKeywordsLabel;
    QBoxLayout *recentKeywordsLayout;
    QLabel *recentChannelsLabel;
    QBoxLayout *recentChannelsLayout;
    QLabel *message;
    QPushButton *watchButton;

};

#endif // __SEARCHVIEW_H__
