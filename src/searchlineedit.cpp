#include "searchlineedit.h"
#include "autocomplete.h"
#include "iconutils.h"

class SearchButton : public QAbstractButton {

public:
    SearchButton(QWidget *parent = 0);
    QMenu *m_menu;

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);

};

SearchButton::SearchButton(QWidget *parent)
    : QAbstractButton(parent),
    m_menu(0) {
    setObjectName(QLatin1String("SearchButton"));
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
}

void SearchButton::mousePressEvent(QMouseEvent *e) {
    if (m_menu && e->button() == Qt::LeftButton) {
        QWidget *p = parentWidget();
        if (p) {
            QPoint r = p->mapToGlobal(QPoint(0, p->height()));
            m_menu->exec(QPoint(r.x() + height() / 2, r.y()));
        }
        e->accept();
    }
    QAbstractButton::mousePressEvent(e);
}

void SearchButton::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter painter(this);
    const int h = height();
    int iconSize = 16;
    if (h > 30) iconSize = 22;
    QPixmap p = IconUtils::icon("edit-find").pixmap(iconSize, iconSize);
    int x = (width() - p.width()) / 2;
    int y = (h - p.height()) / 2;
    painter.drawPixmap(x, y, p);
}

SearchLineEdit::SearchLineEdit(QWidget *parent) : ExLineEdit(parent), searchButton(new SearchButton(this)) {
    connect(m_lineEdit, SIGNAL(textChanged(const QString &)), SIGNAL(textChanged(const QString &)));
    connect(m_lineEdit, SIGNAL(textEdited(const QString &)), SIGNAL(textEdited(const QString &)));
    connect(m_lineEdit, SIGNAL(returnPressed()), SLOT(returnPressed()));

    setLeftWidget(searchButton);
    inactiveText = tr("Search");

    QSizePolicy policy = sizePolicy();
    setSizePolicy(QSizePolicy::Preferred, policy.verticalPolicy());

    // completion
    autoComplete = new AutoComplete(this, m_lineEdit);
    connect(autoComplete, SIGNAL(suggestionAccepted(Suggestion*)), SIGNAL(suggestionAccepted(Suggestion*)));
}

void SearchLineEdit::paintEvent(QPaintEvent *e) {
    ExLineEdit::paintEvent(e);
    if (m_lineEdit->text().isEmpty() && !hasFocus() && !inactiveText.isEmpty()) {
        QStyleOptionFrameV2 panel;
        initStyleOption(&panel);
        QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
        QFontMetrics fm = fontMetrics();
        int horizontalMargin = m_lineEdit->x();
        QRect lineRect(horizontalMargin + r.x(), r.y() + (r.height() - fm.height() + 1) / 2,
                       r.width() - 2 * horizontalMargin, fm.height());
        QPainter painter(this);
        painter.setPen(palette().brush(QPalette::Disabled, QPalette::Text).color());
        painter.drawText(lineRect, Qt::AlignLeft | Qt::AlignVCenter, inactiveText);
    }
}

void SearchLineEdit::resizeEvent(QResizeEvent *e) {
    updateGeometries();
    ExLineEdit::resizeEvent(e);
}

void SearchLineEdit::updateGeometries() {
    int menuHeight = height();
    int menuWidth = menuHeight + 1;
    if (!searchButton->m_menu)
        menuWidth = (menuHeight / 5) * 4;
    searchButton->resize(QSize(menuWidth, menuHeight));
}

void SearchLineEdit::setInactiveText(const QString &text) {
    inactiveText = text;
}

void SearchLineEdit::setText(const QString &text) {
    m_lineEdit->setText(text);
}

AutoComplete *SearchLineEdit::getAutoComplete() {
    return autoComplete;
}

void SearchLineEdit::setMenu(QMenu *menu) {
    if (searchButton->m_menu)
        searchButton->m_menu->deleteLater();
    searchButton->m_menu = menu;
    updateGeometries();
}

QMenu *SearchLineEdit::menu() const {
    if (!searchButton->m_menu) {
        searchButton->m_menu = new QMenu(searchButton);
        if (isVisible())
            (const_cast<SearchLineEdit*>(this))->updateGeometries();
    }
    return searchButton->m_menu;
}

void SearchLineEdit::returnPressed() {
    QString text = m_lineEdit->text().simplified();
    if (!text.isEmpty()) {
        autoComplete->preventSuggest();
        emit search(text);
    }
}

void SearchLineEdit::enableSuggest() {
    autoComplete->enableSuggest();
}

void SearchLineEdit::preventSuggest() {
    autoComplete->preventSuggest();
}

void SearchLineEdit::selectAll() {
    m_lineEdit->selectAll();
}

void SearchLineEdit::setSuggester(Suggester *suggester) {
    autoComplete->setSuggester(suggester);
}

void SearchLineEdit::focusInEvent(QFocusEvent *e) {
    ExLineEdit::focusInEvent(e);
    enableSuggest();
}

void SearchLineEdit::emitTextChanged(const QString &text) {
    autoComplete->enableSuggest();
    emit textEdited(text);
}

QString SearchLineEdit::text() {
    return m_lineEdit->text();
}

QLineEdit *SearchLineEdit::getLineEdit() {
    return m_lineEdit;
}

void SearchLineEdit::setEnabled(bool enabled) {
    ExLineEdit::setEnabled(enabled);
    emit enabledChanged(enabled);
}
