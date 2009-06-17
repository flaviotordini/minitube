#ifndef _THBLACKBAR_H_
#define _THBLACKBAR_H_

#include <QWidget>
class THAction;

class THBlackBar : public QWidget {
	Q_OBJECT

	public:
		THBlackBar (QWidget *parent = 0);
		~THBlackBar();

	public:
		THAction *addAction (THAction *action);
		THAction *addAction (const QString& text);
                void setCheckedAction(int index);

		QSize minimumSizeHint (void) const;

	protected:
		void paintEvent (QPaintEvent *event);

		void mouseMoveEvent (QMouseEvent *event);
                void mousePressEvent (QMouseEvent *event);

	private:
		void drawUnselectedButton (	QPainter *painter,
									const QRect& rect,
									const THAction *action);
		void drawSelectedButton (	QPainter *painter,
									const QRect& rect,
									const THAction *action);
		void drawButton (	QPainter *painter,
							const QRect& rect,
							const THAction *action);
		void drawButton (	QPainter *painter,
							const QRect& rect,
							const QLinearGradient& gradient,
							const QColor& color,
							const THAction *action);

		THAction *hoveredAction (const QPoint& pos) const;
		int calculateButtonWidth (void) const;		

	private:
		class Private;
		Private *d;
};

#endif /* !_THBLACKBAR_H_ */

