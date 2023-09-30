#include "searchlineedit.h"
#include "autocomplete.h"
#include "iconutils.h"

SearchLineEdit::SearchLineEdit(QWidget *parent) : QLineEdit(parent) {
    setPlaceholderText(tr("Search"));

    QAction *searchAction = new QAction();
    IconUtils::setIcon(searchAction, "edit-find");
    addAction(searchAction, QLineEdit::LeadingPosition);

    // completion
    autoComplete = new AutoComplete(this, this);
    connect(autoComplete, SIGNAL(suggestionAccepted(Suggestion *)),
            SIGNAL(suggestionAccepted(Suggestion *)));

    connect(this, SIGNAL(returnPressed()), SLOT(returnPressed()));
}

QMenu *SearchLineEdit::menu() const {
    return nullptr;
}

void SearchLineEdit::setMenu(QMenu *menu) {
    Q_UNUSED(menu);
}

void SearchLineEdit::enableSuggest() {
    autoComplete->enableSuggest();
}

void SearchLineEdit::preventSuggest() {
    autoComplete->preventSuggest();
}

void SearchLineEdit::setSuggester(Suggester *suggester) {
    autoComplete->setSuggester(suggester);
}

AutoComplete *SearchLineEdit::getAutoComplete() {
    return autoComplete;
}

void SearchLineEdit::emitTextChanged(const QString &text) {
    autoComplete->enableSuggest();
    emit QLineEdit::textEdited(text);
}

void SearchLineEdit::returnPressed() {
    QString s = text().simplified();
    if (!s.isEmpty()) {
        autoComplete->preventSuggest();
        emit search(s);
    }
}

QLineEdit *SearchLineEdit::getLineEdit() {
    return this;
}

QWidget *SearchLineEdit::toWidget() {
    return this;
}

void SearchLineEdit::focusInEvent(QFocusEvent *) {
    selectAll();
}

void SearchLineEdit::changeEvent(QEvent *event) {
    if (event->type() == QEvent::FontChange) emit fontChanged();
    QLineEdit::changeEvent(event);
}
