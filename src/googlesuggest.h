#ifndef GOOGLESUGGEST_H
#define GOOGLESUGGEST_H

#include <QtGui>

class GSuggestCompletion : public QObject {
    Q_OBJECT

public:
    GSuggestCompletion(QLineEdit *parent);
    ~GSuggestCompletion();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);

public slots:
    void doneCompletion();
    void preventSuggest();
    void autoSuggest();
    void handleNetworkData(QByteArray response);
    void currentItemChanged(QListWidgetItem *current);

private:
    QLineEdit *editor;
    QString originalText;
    QListWidget *popup;
    QTimer *timer;

};

#endif // GOOGLESUGGEST_H
