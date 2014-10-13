#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include "urllineedit.h"
#include "autocomplete.h"

#include <QLineEdit>
#include <QAbstractButton>

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

class SearchButton;
class Suggester;

/*
    Clear button on the right hand side of the search widget.
    Hidden by default
    "A circle with an X in it"
 */
class ClearButton : public QAbstractButton
{
    Q_OBJECT

public:
    ClearButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);

public slots:
    void textChanged(const QString &text);
};


class SearchLineEdit : public ExLineEdit
{
    Q_OBJECT
    Q_PROPERTY(QString inactiveText READ inactiveText WRITE setInactiveText)

signals:
    void textChanged(const QString &text);
    void textEdited(const QString &text);
    void search(const QString &text);
    void suggestionAccepted(Suggestion *suggestion);

public:
    SearchLineEdit(QWidget *parent = 0);

    QString inactiveText() const;
    void setInactiveText(const QString &text);

    QMenu *menu() const;
    void setMenu(QMenu *menu);
    void updateGeometries();
    void enableSuggest();
    void preventSuggest();
    void selectAll() { lineEdit()->selectAll(); }
    void setSuggester(Suggester *suggester) { completion->setSuggester(suggester); }
    void setText(const QString &text) { lineEdit()->setText(text); }

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void focusInEvent(QFocusEvent *event);

private slots:
    void returnPressed();

private:
    SearchButton *m_searchButton;
    QString m_inactiveText;
    AutoComplete *completion;
};

#endif // SEARCHLINEEDIT_H

