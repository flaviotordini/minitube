#ifndef _THACTION_H_
#define _THACTION_H_

#include <QObject>
#include <QIcon>

class THAction : public QObject {
	Q_OBJECT

	Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
	Q_PROPERTY(QString text READ text WRITE setText)

	public:
		THAction (QObject *parent = 0);
		THAction (const QString& text, QObject *parent = 0);
		THAction (const QIcon& icon, const QString& text, QObject *parent = 0);
		~THAction();

	signals:
		void triggered (bool checked = false);
		void toggled (bool checked);
		void hovered (void);

	public:
		bool isChecked (void) const;
		bool isHovered (void) const;

		QIcon icon (void) const;
		void setIcon (const QIcon& icon);

		QString text (void) const;
		void setText (const QString& text);

	public slots:
		void toggle (void);
		void trigger (void);
		void hover (bool isHovered = false);
		void setChecked (bool checked);

	private:
		class Private;
		Private *d;
};

#endif /* !_THACTION_H_ */

