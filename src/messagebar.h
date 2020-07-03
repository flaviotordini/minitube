#ifndef MESSAGEBAR_H
#define MESSAGEBAR_H

#include <QtWidgets>

class MessageBar : public QWidget {
    Q_OBJECT

public:
    MessageBar(QWidget *parent = 0);
    void setMessage(const QString &message);
    void setOpenExternalLinks(bool value);

signals:
    void linkActivated(const QString &link);
    void closed();

protected:
    void paintEvent(QPaintEvent *e);

private:
    QLabel *msgLabel;
};

#endif // MESSAGEBAR_H
