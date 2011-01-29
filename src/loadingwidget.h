#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QtGui>
#include "video.h"

class LoadingWidget : public QWidget {

    Q_OBJECT

public:
    LoadingWidget(QWidget *parent);
    void setVideo(Video *video);
    void setError(QString message);
    void clear();

public slots:
    void bufferStatus(int);

private:
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    QProgressBar *progressBar;
    QTime startTime;

};

#endif // LOADINGWIDGET_H
