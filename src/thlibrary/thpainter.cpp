#include "thpainter.h"
#include "thimage.h"

/* ============================================================================
 *  PUBLIC Constructor/Destructors
 */
THPainter::THPainter() 
	: QPainter()
{
	setRenderHint(QPainter::Antialiasing);
	setRenderHint(QPainter::TextAntialiasing);
}

THPainter::THPainter(QPaintDevice *device)
	: QPainter(device)
{
	setRenderHint(QPainter::Antialiasing);
	setRenderHint(QPainter::TextAntialiasing);
}

THPainter::~THPainter() {
}

/* ============================================================================
 *  PUBLIC STATIC Methods
 */
QPainterPath THPainter::roundRectangle (const QRectF& rect, qreal radius) {
	return(roundRectangle(rect, radius, radius, radius, radius));
}

QPainterPath THPainter::roundRectangle (const QRectF& rect, 
										qreal leftRadius,
										qreal rightRadius)
{
	return(roundRectangle(rect, leftRadius, leftRadius, rightRadius, rightRadius));
}

QPainterPath THPainter::roundRectangle (const QRectF& rect, 
										qreal leftTopRadius,
										qreal leftBottomRadius,
										qreal rightTopRadius,
										qreal rightBottomRadius)
{
	// Top Left Corner
	// Top Side
	// Top right corner
	// Right side
	// Bottom right corner
	// Bottom side
	// Bottom left corner

	QPainterPath path(QPoint(rect.left(), rect.top() + leftTopRadius));
	path.quadTo(rect.left(), rect.top(), rect.left() + leftTopRadius, rect.top());
	path.lineTo(rect.right() - rightTopRadius, rect.top());
	path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + rightTopRadius);
	path.lineTo(rect.right(), rect.bottom() - rightBottomRadius);
	path.quadTo(rect.right(), rect.bottom(), rect.right() - rightBottomRadius, rect.bottom());
	path.lineTo(rect.left() + leftBottomRadius, rect.bottom());
	path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - leftBottomRadius);
	path.closeSubpath();

	return(path);
}

/* ============================================================================
 *  PUBLIC Draw Methods
 */
void THPainter::drawRoundRect (const QRectF& rect, qreal radius) {
	drawPath(roundRectangle(rect, radius, radius, radius, radius));
}

void THPainter::drawRoundRect (const QRectF& rect, 
									qreal leftRadius,
									qreal rightRadius) 
{
	drawPath(roundRectangle(rect, leftRadius, leftRadius,
							rightRadius, rightRadius));
}

void THPainter::drawRoundRect (	const QRectF& rect,
								qreal leftTopRadius,
								qreal leftBottomRadius,
								qreal rightTopRadius,
								qreal rightBottomRadius)
{
	drawPath(roundRectangle(rect, leftTopRadius, leftBottomRadius,
							rightTopRadius, rightBottomRadius));
}

void THPainter::drawShadowText (qreal x, qreal y,
								const QString& text,
								const QColor& shadowColor,
								const QPointF& offset,
								qreal radius)
{
	QPainter p;

	// Draw Text
	QRect textRect = QFontMetrics(text).boundingRect(text);
	QImage textImage(textRect.size(), QImage::Format_ARGB32_Premultiplied);
	textImage.fill(Qt::transparent);
	p.begin(&textImage);
	p.setPen(pen());
	p.setFont(font());
	p.drawText(textImage.rect(), Qt::AlignLeft, text);
	p.end();

	// Draw Blurred Shadow
	THImage shadowImage(textRect.size() + QSize(radius * 2, radius * 2),
						QImage::Format_ARGB32_Premultiplied);
	shadowImage.fill(Qt::transparent);
	p.begin(&shadowImage);
	p.drawImage(radius, radius, textImage);
	p.end();
	shadowImage.shadowBlur(radius, shadowColor);

	// Compose Text and Shadow
	int addSizeX = (offset.x() > radius) ? (abs(offset.x()) - radius) : 0;
	int addSizeY = (offset.y() > radius) ? (abs(offset.y()) - radius) : 0;
	QSize finalSize = shadowImage.size() + QSize(addSizeX, addSizeY);

	QPointF shadowTopLeft(QPointF((finalSize.width() - shadowImage.width()) / 2.0,
							(finalSize.height() - shadowImage.height()) / 2.0) +
							(offset / 2.0));
	QPointF textTopLeft(QPointF((finalSize.width() - textImage.width()) / 2.0,
						((finalSize.height() - textImage.height()) / 2.0)) - 
						(offset / 2.0));

	// Paint Text and Shadow
	save();
	translate(x, y);
	setCompositionMode(QPainter::CompositionMode_Xor);
	drawImage(shadowTopLeft, shadowImage);
	drawImage(textTopLeft, textImage);
	restore();
}

/* ============================================================================
 *  PUBLIC Fill Methods
 */
void THPainter::fillRoundRect (	const QRectF& rect,
								qreal radius,
								const QBrush& brush)
{
	fillPath(roundRectangle(rect, radius, radius, radius, radius), brush);
}

void THPainter::fillRoundRect (	const QRectF& rect,
								qreal leftRadius,
								qreal rightRadius,
								const QBrush& brush)
{
	fillPath(roundRectangle(rect, leftRadius, leftRadius, 
							rightRadius, rightRadius), brush);
}

void THPainter::fillRoundRect (	const QRectF& rect,
								qreal leftTopRadius,
								qreal leftBottomRadius,
								qreal rightTopRadius,
								qreal rightBottomRadius,
								const QBrush& brush)
{
	fillPath(roundRectangle(rect, leftTopRadius, leftBottomRadius, 
							rightTopRadius, rightBottomRadius), brush);
}

