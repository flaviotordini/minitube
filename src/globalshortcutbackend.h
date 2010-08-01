#ifndef GLOBALSHORTCUTBACKEND_H
#define GLOBALSHORTCUTBACKEND_H

#include <QObject>

class GlobalShortcuts;

class GlobalShortcutBackend : public QObject {
public:
  GlobalShortcutBackend(GlobalShortcuts* parent = 0);
  virtual ~GlobalShortcutBackend() {}

  bool is_active() const { return active_; }

  bool Register();
  void Unregister();
  void Reregister();

protected:
  virtual bool DoRegister() = 0;
  virtual void DoUnregister() = 0;

  GlobalShortcuts* manager_;
  bool active_;
};

#endif // GLOBALSHORTCUTBACKEND_H
