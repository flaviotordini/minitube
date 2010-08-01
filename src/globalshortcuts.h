#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

#include <QtCore>
#include <QAction>

class GlobalShortcutBackend;

class GlobalShortcuts : public QObject {

    Q_OBJECT

public:
    static GlobalShortcuts& instance();

    struct Shortcut {
        QString id;
        QKeySequence default_key;
        QAction* action;
    };

    QMap<QString, Shortcut> shortcuts() const { return shortcuts_; }
    void setBackend(GlobalShortcutBackend* backend) {
        this->backend = backend;
        reload();
    }

public slots:
    void reload();

signals:
    void Play();
    void Pause();
    void PlayPause();
    void Stop();
    void StopAfter();
    void Next();
    void Previous();
    void IncVolume();
    void DecVolume();
    void Mute();
    void SeekForward();
    void SeekBackward();

private:
    GlobalShortcuts(QObject* parent = 0);
    void AddShortcut(const QString& id, const QString& name, const char* signal,
                     const QKeySequence& default_key = QKeySequence(0));

private:
    GlobalShortcutBackend* backend;

    QMap<QString, Shortcut> shortcuts_;
};

#endif
