#ifndef _THBLACKBUTTON_H_
#define _THBLACKBUTTON_H_

#include <QAbstractButton>

class THBlackButton : public QAbstractButton {
	Q_OBJECT

	public:
		THBlackButton (QWidget *parent = 0);
		THBlackButton (const QString& text, QWidget *parent = 0);
		~THBlackButton();

	public:
		void setRadius (qreal radius);
		void setRadius (qreal leftTopRadius,
						qreal leftBottomRadius,
						qreal rightTopRadius,
						qreal rightBottomRadius);

		QSize minimumSizeHint (void) const;

	protected:
		void paintEvent (QPaintEvent *event);

	private:
		class Private;
		Private *d;
};

#endif /* !_THBLACKBUTTON_H_ */

