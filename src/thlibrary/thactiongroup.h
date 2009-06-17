#ifndef _THACTIONGROUP_H_
#define _THACTIONGROUP_H_

#include <QObject>
#include <QIcon>
class THAction;

class THActionGroup : public QObject {
	Q_OBJECT

	public:
		THActionGroup (QObject *parent = 0);
		THActionGroup (const QString& name, QObject *parent = 0);
		~THActionGroup();

	public:
		THAction *addAction (THAction *action);
		THAction *addAction (const QString& text);
		THAction *addAction (const QIcon& icon, const QString& text);

		QString name (void) const;
		void setName (const QString& name);

	private:
		class Private;
		Private *d;
};

#endif /* !_THACTIONGROUP_H_ */

