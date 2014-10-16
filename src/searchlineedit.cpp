#include "searchlineedit.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QStyle>
#include <QStyleOptionFrameV2>

#include "autocomplete.h"
#include "iconutils.h"

ClearButton::ClearButton(QWidget *parent) : QAbstractButton(parent), hovered(false), mousePressed(false) {
    setCursor(Qt::ArrowCursor);
    setToolTip(tr("Clear"));
    setVisible(false);
    setFocusPolicy(Qt::NoFocus);
}

void ClearButton::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter painter(this);
    const int h = height();
    int iconSize = 16;
    if (h > 30) iconSize = 22;
    QIcon::Mode iconMode = QIcon::Normal;
    if (mousePressed) iconMode = QIcon::Active;
    QPixmap p = IconUtils::icon("edit-clear").pixmap(iconSize, iconSize, iconMode);
    // QPixmap p = IconUtils::tintedIcon("edit-clear", Qt::black, QSize(iconSize, iconSize)).pixmap(iconSize, iconSize, iconMode);
    int x = (width() - p.width()) / 2;
    int y = (h - p.height()) / 2;
    painter.drawPixmap(x, y, p);
}

void ClearButton::textChanged(const QString &text) {
    setVisible(!text.isEmpty());
}

void ClearButton::enterEvent(QEvent *e) {
    Q_UNUSED(e);
    hovered = true;
}

void ClearButton::leaveEvent(QEvent *e) {
    Q_UNUSED(e);
    hovered = false;
}

void ClearButton::mousePressEvent(QEvent *e) {
    Q_UNUSED(e);
    mousePressed = true;
}

void ClearButton::mouseReleaseEvent(QEvent *e) {
    Q_UNUSED(e);
    mousePressed = false;
}

/*
    Search icon on the left hand side of the search widget
    When a menu is set a down arrow appears
 */
class SearchButton : public QAbstractButton {
public:
    SearchButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    QMenu *m_menu;

protected:
    void mousePressEvent(QMouseEvent *event);
};

SearchButton::SearchButton(QWidget *parent)
    : QAbstractButton(parent),
    m_menu(0) {
    setObjectName(QLatin1String("SearchButton"));
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
}

void SearchButton::mousePressEvent(QMouseEvent *event) {
    if (m_menu && event->button() == Qt::LeftButton) {
        QWidget *p = parentWidget();
        if (p) {
            QPoint r = p->mapToGlobal(QPoint(0, p->height()));
            m_menu->exec(QPoint(r.x() + height() / 2, r.y()));
        }
        event->accept();
    }
    QAbstractButton::mousePressEvent(event);
}

void SearchButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    const int h = height();
    int iconSize = 16;
    if (h > 30) iconSize = 22;
    QPixmap p = IconUtils::fromTheme("edit-find-symbolic").pixmap(iconSize, iconSize);
    int x = (width() - p.width()) / 2;
    int y = (h - p.height()) / 2;
    painter.drawPixmap(x, y, p);
}

/*
    SearchLineEdit is an enhanced QLineEdit
    - A Search icon on the left with optional menu
    - When there is no text and doesn't have focus an "inactive text" is displayed
    - When there is text a clear button is displayed on the right hand side
 */
SearchLineEdit::SearchLineEdit(QWidget *parent) : ExLineEdit(parent),
searchButton(new SearchButton(this)) {
    connect(lineEdit(), SIGNAL(textChanged(const QString &)), SIGNAL(textChanged(const QString &)));
    connect(lineEdit(), SIGNAL(textEdited(const QString &)), SIGNAL(textEdited(const QString &)));
    connect(lineEdit(), SIGNAL(returnPressed()), SLOT(returnPressed()));

    setLeftWidget(searchButton);
    inactiveText = tr("Search");

    QSizePolicy policy = sizePolicy();
    setSizePolicy(QSizePolicy::Preferred, policy.verticalPolicy());

    // completion
    autoComplete = new AutoComplete(this, m_lineEdit);
    connect(autoComplete, SIGNAL(suggestionAccepted(Suggestion*)), SIGNAL(suggestionAccepted(Suggestion*)));
}

void SearchLineEdit::paintEvent(QPaintEvent *event) {
    if (lineEdit()->text().isEmpty() && !hasFocus() && !inactiveText.isEmpty()) {
        ExLineEdit::paintEvent(event);
        QStyleOptionFrameV2 panel;
        initStyleOption(&panel);
        QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
        QFontMetrics fm = fontMetrics();
        int horizontalMargin = lineEdit()->x();
        QRect lineRect(horizontalMargin + r.x(), r.y() + (r.height() - fm.height() + 1) / 2,
                       r.width() - 2 * horizontalMargin, fm.height());
        QPainter painter(this);
        painter.setPen(palette().brush(QPalette::Disabled, QPalette::Text).color());
        painter.drawText(lineRect, Qt::AlignLeft|Qt::AlignVCenter, inactiveText);
    } else {
        ExLineEdit::paintEvent(event);
    }
}

void SearchLineEdit::resizeEvent(QResizeEvent *event) {
    updateGeometries();
    ExLineEdit::resizeEvent(event);
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
    if (!lineEdit()->text().isEmpty()) {
        autoComplete->preventSuggest();
        emit search(lineEdit()->text());
    }
}

void SearchLineEdit::enableSuggest() {
    autoComplete->enableSuggest();
}

void SearchLineEdit::preventSuggest() {
    autoComplete->preventSuggest();
}

void SearchLineEdit::focusInEvent(QFocusEvent *event) {
    ExLineEdit::focusInEvent(event);
    enableSuggest();
}
