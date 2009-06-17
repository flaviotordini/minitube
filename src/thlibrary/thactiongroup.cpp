#include <QList>

#include "thactiongroup.h"
#include "thaction.h"

/* ============================================================================
 *  PRIVATE Class
 */
class THActionGroup::Private {
	public:
		QList<THAction *> actionList;
		QString name;
};

/* ============================================================================
 *  PUBLIC Constructor/Destructors
 */
THActionGroup::THActionGroup (QObject *parent) 
	: QObject(parent), d(new THActionGroup::Private)
{
}

THActionGroup::THActionGroup (const QString& name, QObject *parent)
	: QObject(parent), d(new THActionGroup::Private)
{
	d->name = name;
}

THActionGroup::~THActionGroup() {
	delete d;
}

/* ============================================================================
 *  PUBLIC Methods
 */
THAction *THActionGroup::addAction (THAction *action) {
	d->actionList.append(action);
	return(action);
}

THAction *THActionGroup::addAction (const QString& text) {
	THAction *action = new THAction(text, this);
	d->actionList.append(action);
	return(action);
}

THAction *THActionGroup::addAction (const QIcon& icon, const QString& text) {
	THAction *action = new THAction(icon, text, this);
	d->actionList.append(action);
	return(action);
}

/* ============================================================================
 *  PUBLIC Properties
 */
QString THActionGroup::name (void) const {
	return(d->name);
}

void THActionGroup::setName (const QString& name) {
	d->name = name;
}

