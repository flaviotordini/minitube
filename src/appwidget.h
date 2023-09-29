#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QtWidgets>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef QEnterEvent CompatibleEnterEvent;
#else
typedef QEvent CompatibleEnterEvent;
#endif

class AppWidget : public QWidget {
    Q_OBJECT

public:
    AppWidget(const QString &name, const QString &code, QWidget *parent = 0);
    QLabel *icon;

protected:
    void enterEvent(CompatibleEnterEvent *e);
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
