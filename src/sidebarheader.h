#ifndef SIDEBARHEADER_H
#define SIDEBARHEADER_H

#include <QtGui>

class SidebarHeader : public QToolBar {

    Q_OBJECT

public:
    SidebarHeader(QWidget *parent = 0);
    void updateInfo();

protected:
    QSize minimumSizeHint() const;
    void paintEvent(QPaintEvent *event);

private slots:
    void updateTitle(QString title);

private:
    void setup();
    void setTitle(QString title);

    QAction *backAction;
    QAction * forwardAction;
    QString title;
};

#endif // SIDEBARHEADER_H
