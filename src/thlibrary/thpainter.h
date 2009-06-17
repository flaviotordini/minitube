#ifndef _THPAINTER_H_
#define _THPAINTER_H_

#include <QPainter>

class THPainter : public QPainter {
	public:
		THPainter();
		THPainter(QPaintDevice *device);
		~THPainter();

		// STATIC Methods
		static QPainterPath roundRectangle (const QRectF& rect, qreal radius);
		static QPainterPath roundRectangle (const QRectF& rect, 
											qreal leftRadius,
											qreal rightRadius);
		static QPainterPath roundRectangle (const QRectF& rect, 
											qreal leftTopRadius,
											qreal leftBottomRadius,
											qreal rightTopRadius,
											qreal rightBottomRadius);

		// Methods
		void drawShadowText (	qreal x, qreal y,
								const QString& text,
								const QColor& shadowColor,
								const QPointF& offset,
								qreal radius);

		void drawRoundRect (const QRectF& rect,
							qreal radius);
		void drawRoundRect (const QRectF& rect,
							qreal leftRadius,
							qreal rightRadius);
		void drawRoundRect (const QRectF& rect,
							qreal leftTopRadius,
							qreal leftBottomRadius,
							qreal rightTopRadius,
							qreal rightBottomRadius);

		void fillRoundRect (const QRectF& rect,
							qreal radius,
							const QBrush& brush);
		void fillRoundRect (const QRectF& rect,
							qreal leftRadius,
							qreal rightRadius,
							const QBrush& brush);
		void fillRoundRect (const QRectF& rect,
							qreal leftTopRadius,
							qreal leftBottomRadius,
							qreal rightTopRadius,
							qreal rightBottomRadius,
							const QBrush& brush);
};

#endif /* !_THPAINTER_H_ */

