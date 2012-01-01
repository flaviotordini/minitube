#ifndef DOWNLOADSETTINGS_H
#define DOWNLOADSETTINGS_H

#include <QtGui>

class DownloadSettings : public QWidget {

    Q_OBJECT

public:
    DownloadSettings(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void changeFolder();
    void folderChosen(const QString &folder);

private:
    void updateMessage();

    QLabel *message;
    QPushButton *changeFolderButton;

};

#endif // DOWNLOADSETTINGS_H
