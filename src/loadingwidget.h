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
    // TODO uncomment the whole progress bar feature
    // when the Phonon backends will correctly emit bufferStatus(int)
    // QProgressBar *progressBar;

};

#endif // LOADINGWIDGET_H
