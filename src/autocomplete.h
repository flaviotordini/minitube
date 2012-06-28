#ifndef SUGGESTCOMPLETION_H
#define SUGGESTCOMPLETION_H

#include <QtGui>

class Suggester;
class SearchLineEdit;

class AutoComplete : public QObject {
    Q_OBJECT

public:
    AutoComplete(SearchLineEdit *parent, QLineEdit *editor);
    ~AutoComplete();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);
    void setSuggester(Suggester* suggester);
    QListWidget* getPopup() { return popup; }

public slots:
    void doneCompletion();
    void preventSuggest();
    void enableSuggest();
    void autoSuggest();
    void currentItemChanged(QListWidgetItem *current);
    void suggestionsReady(QStringList suggestions);

signals:
    void suggestionAccepted(const QString &suggestion);

private:
    SearchLineEdit *buddy;
    QLineEdit *editor;
    QString originalText;
    QListWidget *popup;
    QTimer *timer;
    bool enabled;
    Suggester* suggester;

};

#endif // SUGGESTCOMPLETION_H
