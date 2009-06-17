#include <QPaintEvent>

#include "thblackbutton.h"
#include "thpainter.h"

/* ============================================================================
 *  PRIVATE Class
 */
class THBlackButton::Private {
	public:
		qreal leftTopRadius;
		qreal leftBottomRadius;
		qreal rightTopRadius;
		qreal rightBottomRadius;
};

/* ============================================================================
 *  PUBLIC Constructor/Destructors
 */
THBlackButton::THBlackButton (QWidget *parent)
	: QAbstractButton(parent), d(new THBlackButton::Private)
{
	setRadius(10);
}

THBlackButton::THBlackButton (const QString& text, QWidget *parent)
	: QAbstractButton(parent), d(new THBlackButton::Private)
{
	setRadius(10);
	setText(text);
}

THBlackButton::~THBlackButton() {
	delete d;
}

/* ============================================================================
 *  PUBLIC Methods
 */
void THBlackButton::setRadius (qreal radius) {
	d->leftTopRadius = radius;
	d->leftBottomRadius = radius;
	d->rightTopRadius = radius;
	d->rightBottomRadius = radius;
}

void THBlackButton::setRadius (	qreal leftTopRadius,
								qreal leftBottomRadius,
								qreal rightTopRadius,
								qreal rightBottomRadius)
{
	d->leftTopRadius = leftTopRadius;
	d->leftBottomRadius = leftBottomRadius;
	d->rightTopRadius = rightTopRadius;
	d->rightBottomRadius = rightBottomRadius;
}

QSize THBlackButton::minimumSizeHint (void) const {
	QFontMetrics fontMetrics(QFont("Arial", 8, QFont::Bold));
	int width = fontMetrics.width(text()) + 48;
	return(QSize(width, 22));
}

/* ============================================================================
 *  PROTECTED Methods
 */
void THBlackButton::paintEvent (QPaintEvent *event) {
	int height = event->rect().height();
	int width = event->rect().width() - 10;
	int mh = (height / 2);

	THPainter p(this);
	p.setPen(QPen(QColor(0x28, 0x28, 0x28), 1));
	
	p.translate(5, 0);

	QLinearGradient linearGrad;
	QColor color;
	if (isDown()) {
		linearGrad = QLinearGradient(QPointF(0, 0), QPointF(0, mh));
		linearGrad.setColorAt(0, QColor(0x6c, 0x6c, 0x6c));
		linearGrad.setColorAt(1, QColor(0x40, 0x40, 0x40));
		color = QColor(0x35, 0x35, 0x35);
	} else {
		linearGrad = QLinearGradient(QPointF(0, 0), QPointF(0, mh));
		linearGrad.setColorAt(0, QColor(0x8e, 0x8e, 0x8e));
		linearGrad.setColorAt(1, QColor(0x5c, 0x5c, 0x5c));
		color = QColor(0x41, 0x41, 0x41);
	}

	p.fillRoundRect(QRect(0, 0, width, mh), 
					d->leftTopRadius, 0, d->rightTopRadius, 0, 
					QBrush(linearGrad));
	p.fillRoundRect(QRect(0, mh, width, mh), 
					0, d->leftBottomRadius, 0, d->rightBottomRadius, 
					color);
	p.drawRoundRect(QRect(0, 0, width, height), 
					d->leftTopRadius, d->leftBottomRadius,
					d->rightTopRadius, d->rightBottomRadius);

	p.translate(-5, 0);
	
	p.setFont(QFont("Arial", 8, QFont::Bold));
	p.setPen(QPen(QColor(0xff, 0xff, 0xff), 1));
	p.drawText(event->rect(), Qt::AlignCenter, text());

	p.end();
}

