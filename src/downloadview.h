#ifndef DOWNLOADVIEW_H
#define DOWNLOADVIEW_H

#include <QtGui>
#include "View.h"

class SegmentedControl;
class DownloadModel;
class DownloadListView;
class DownloadSettings;

class DownloadView : public QWidget, public View {

    Q_OBJECT

public:
    DownloadView(QWidget *parent);
    void appear();
    void disappear();
    QMap<QString, QVariant> metadata() {
        QMap<QString, QVariant> metadata;
        metadata.insert("title", tr("Downloads"));
        metadata.insert("description", "");
        return metadata;
    }

public slots:
    void itemEntered(const QModelIndex &index);
    void buttonPushed(QModelIndex index);

private:
    SegmentedControl *bar;
    DownloadListView *listView;
    DownloadModel *listModel;
    QTimer *updateTimer;
    DownloadSettings *downloadSettings;

};

#endif // DOWNLOADVIEW_H
