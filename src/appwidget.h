#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QtWidgets>


class AppWidget : public QWidget {

    Q_OBJECT

public:
    AppWidget(const QString &name, const QString &code, QWidget *parent = 0);
    QLabel *icon;

protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private slots:
    void iconDownloaded(const QByteArray &bytes);
    void downloadApp();

private:
    QPushButton *downloadButton;
    QString name;
    QString url;
    QString webPage;
};

class AppsWidget : public QWidget {

    Q_OBJECT

public:
    AppsWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *e);

private:
    void setupApp(const QString &name, const QString &code);

};

#endif // APPWIDGET_H
