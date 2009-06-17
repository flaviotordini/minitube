#include "thaction.h"

/* ============================================================================
 *  PRIVATE Class
 */
class THAction::Private {
	public:
		bool isHovered;
		bool isChecked;
		QString text;
		QIcon icon;
};

/* ============================================================================
 *  PUBLIC Constructor/Destructors
 */
THAction::THAction (QObject *parent)
	: QObject(parent), d(new THAction::Private)
{
	d->isHovered = false;
	d->isChecked = false;
}

THAction::THAction (const QString& text, QObject *parent) 
	: QObject(parent), d(new THAction::Private)
{
	d->isHovered = false;
	d->isChecked = false;
	d->text = text;
}

THAction::THAction (const QIcon& icon, const QString& text, QObject *parent)
	: QObject(parent), d(new THAction::Private)
{
	d->isHovered = false;
	d->isChecked = false;
	d->icon = icon;
	d->text = text;
}

THAction::~THAction() {
	delete d;
}

/* ============================================================================
 *  PUBLIC Properties
 */
bool THAction::isChecked (void) const {
	return(d->isChecked);
}

bool THAction::isHovered (void) const {
	return(d->isHovered);
}

QIcon THAction::icon (void) const {
	return(d->icon);
}

void THAction::setIcon (const QIcon& icon) {
	d->icon = icon;
}


QString THAction::text (void) const {
	return(d->text);
}

void THAction::setText (const QString& text) {
	d->text = text;
}

/* ============================================================================
 *  PUBLIC SLOTS
 */
void THAction::hover (bool isHovered) {
	d->isHovered = isHovered;
	if (d->isHovered) emit hovered();
}

void THAction::toggle (void) {
	emit toggled(d->isChecked);
}

void THAction::trigger (void) {
	emit triggered(d->isChecked);
}

void THAction::setChecked (bool checked) {
	d->isChecked = checked;
}
