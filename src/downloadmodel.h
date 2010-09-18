#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractListModel>

class DownloadManager;

class DownloadModel : public QAbstractListModel {

    Q_OBJECT

public:
    DownloadModel(DownloadManager *downloadManager, QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    friend class DownloadManager;
    void setHoveredRow(int row);

public slots:
    void clearHover();
    void enterPlayIconHover();
    void exitPlayIconHover();
    void enterPlayIconPressed();
    void exitPlayIconPressed();
    void sendReset();
    void updatePlayIcon();

private:
    int columnCount() { return 1; }

    DownloadManager *downloadManager;
    int hoveredRow;
    bool playIconHovered;
    bool playIconPressed;

};

#endif // DOWNLOADMODEL_H
