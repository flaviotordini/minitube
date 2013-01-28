#ifndef __SEARCHVIEW_H__
#define __SEARCHVIEW_H__

#include <QtGui>
#include "view.h"

class SearchLineEdit;
class SearchParams;
class YTSuggester;
class ChannelSuggest;

class SearchView : public QWidget, public View {

    Q_OBJECT

public:
    SearchView(QWidget *parent = 0);
    void updateRecentKeywords();
    void updateRecentChannels();

public slots:
    void appear();
    void disappear() { }
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
    YTSuggester *youtubeSuggest;
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
