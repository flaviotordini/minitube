#include "exlineedit.h"
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

void ClearButton::mousePressEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    mousePressed = true;
}

void ClearButton::mouseReleaseEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    mousePressed = false;
}

ExLineEdit::ExLineEdit(QWidget *parent)
    : QWidget(parent)
    , m_leftWidget(0)
    , m_lineEdit(new QLineEdit(this))
    , m_clearButton(new ClearButton(this)) {
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
    setSizePolicy(m_lineEdit->sizePolicy());
    setBackgroundRole(m_lineEdit->backgroundRole());
    setMouseTracking(true);
    setAcceptDrops(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);
    QPalette p = m_lineEdit->palette();
    setPalette(p);

    // line edit
    m_lineEdit->setFrame(false);
    m_lineEdit->setFocusProxy(this);
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_lineEdit->setStyleSheet("background:transparent");
    QPalette clearPalette = m_lineEdit->palette();
    clearPalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    m_lineEdit->setPalette(clearPalette);

    // clearButton
    connect(m_clearButton, SIGNAL(clicked()), m_lineEdit, SLOT(clear()));
    connect(m_lineEdit, SIGNAL(textChanged(const QString&)), m_clearButton, SLOT(textChanged(const QString&)));
}

void ExLineEdit::setFont(const QFont &font) {
    m_lineEdit->setFont(font);
    updateGeometries();
}

void ExLineEdit::setLeftWidget(QWidget *widget) {
    m_leftWidget = widget;
}

QWidget *ExLineEdit::leftWidget() const {
    return m_leftWidget;
}

void ExLineEdit::clear() {
    m_lineEdit->clear();
}

QString ExLineEdit::text() {
    return m_lineEdit->text();
}

void ExLineEdit::resizeEvent(QResizeEvent *e) {
    Q_ASSERT(m_leftWidget);
    updateGeometries();
    QWidget::resizeEvent(e);
}

void ExLineEdit::updateGeometries() {
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    QRect rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

    int padding = 3;
    // int height = rect.height() + padding*2;
    int width = rect.width();

    // int m_leftWidgetHeight = m_leftWidget->height();
    m_leftWidget->setGeometry(rect.x() + 2,          0,
                              m_leftWidget->width(), m_leftWidget->height());

    int clearButtonWidth = this->height();
    m_lineEdit->setGeometry(m_leftWidget->x() + m_leftWidget->width(),        padding,
                            width - clearButtonWidth - m_leftWidget->width(), this->height() - padding*2);

    m_clearButton->setGeometry(this->width() - clearButtonWidth, 0,
                               clearButtonWidth, this->height());
}

void ExLineEdit::initStyleOption(QStyleOptionFrameV2 *option) const {
    option->initFrom(this);
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    if (m_lineEdit->isReadOnly())
        option->state |= QStyle::State_ReadOnly;
#ifdef QT_KEYPAD_NAVIGATION
    if (hasEditFocus())
        option->state |= QStyle::State_HasEditFocus;
#endif
    option->features = QStyleOptionFrameV2::None;
}

QSize ExLineEdit::sizeHint() const {
    m_lineEdit->setFrame(true);
    QSize size = m_lineEdit->sizeHint();
    m_lineEdit->setFrame(false);
    size = size + QSize(3, 3);
    return size;
}

void ExLineEdit::focusInEvent(QFocusEvent *e) {
    m_lineEdit->event(e);
    QWidget::focusInEvent(e);
}

void ExLineEdit::focusOutEvent(QFocusEvent *e) {
    m_lineEdit->event(e);

    if (m_lineEdit->completer()) {
        connect(m_lineEdit->completer(), SIGNAL(activated(QString)),
                         m_lineEdit, SLOT(setText(QString)));
        connect(m_lineEdit->completer(), SIGNAL(highlighted(QString)),
                         m_lineEdit, SLOT(_q_completionHighlighted(QString)));
    }
    QWidget::focusOutEvent(e);
}

void ExLineEdit::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape && !m_lineEdit->text().isEmpty()) {
        m_lineEdit->clear();
    }
    m_lineEdit->event(e);
    QWidget::keyPressEvent(e);
}

bool ExLineEdit::event(QEvent *e) {
    if (e->type() == QEvent::ShortcutOverride || e->type() == QEvent::InputMethod)
        m_lineEdit->event(e);
    return QWidget::event(e);
}

void ExLineEdit::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter p(this);
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);
}
