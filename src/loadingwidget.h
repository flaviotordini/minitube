#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QtGui>
#include "video.h"

class LoadingWidget : public QWidget {

    Q_OBJECT

public:
    LoadingWidget(QWidget *parent);
    void setVideo(Video *video);

public slots:
    void bufferStatus(int);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    QProgressBar *progressBar;

};

#endif // LOADINGWIDGET_H
