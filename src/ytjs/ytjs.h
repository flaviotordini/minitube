#ifndef YTJS_H
#define YTJS_H

#include <QJSEngine>
#include <QQmlEngine>
#include <QtCore>

class Http;

class JsTimer : public QTimer {
    Q_OBJECT

public:
    static auto &getTimers() {
        static QHash<QString, JsTimer *> timers;
        return timers;
    }
    // This should be static but cannot bind static functions to QJSEngine
    Q_INVOKABLE QJSValue clearTimeout(QJSValue id) {
        // qDebug() << id.toString();
        auto timer = getTimers().take(id.toString());
        if (timer) {
            timer->stop();
            timer->deleteLater();
        }
        return QJSValue();
    }
    // This should be static but cannot bind static functions to QJSEngine
    Q_INVOKABLE QJSValue setTimeout(QJSValue callback, QJSValue delayTime) {
        // qDebug() << callback.toString() << delayTime.toInt();
        auto timer = new JsTimer();
        timer->setInterval(delayTime.toInt());
        connect(timer, &JsTimer::timeout, this, [callback]() mutable {
            // qDebug() << "Calling" << callback.toString();
            auto value = callback.call();
            if (value.isError()) {
                qWarning() << "Error" << value.toString();
                qDebug() << value.property("stack").toString().splitRef('\n');
            }
        });
        timer->start();
        return timer->hashString();
    }

    Q_INVOKABLE JsTimer(QObject *parent = nullptr) : QTimer(parent) {
        setTimerType(Qt::VeryCoarseTimer);
        setSingleShot(true);
        connect(this, &JsTimer::destroyed, this, [this] { getTimers().remove(hashString()); });
        connect(this, &JsTimer::timeout, this, &QTimer::deleteLater);
        getTimers().insert(hashString(), this);
    }

    QString hashString() { return QString::number((std::uintptr_t)(this)); }

private:
};

class ResultHandler : public QObject {
    Q_OBJECT

public:
    Q_INVOKABLE QJSValue setData(QJSValue value) {
        qDebug() << "Success" << value.toString();
        auto doc = QJsonDocument::fromVariant(value.toVariant());
        if (doc.isEmpty()) {
            qDebug() << value.toString();
            emit error("Cannot parse JSON");
            return QJSValue();
        }
        emit data(doc);
        return QJSValue();
    }

    Q_INVOKABLE QJSValue setError(QJSValue value) {
        QString message = value.toString();
        qWarning() << "Error" << message;
        qDebug() << value.property("stack").toString().splitRef('\n');
        emit error(message);
        return QJSValue();
    }

signals:
    void data(const QJsonDocument &doc);
    void error(const QString &message);
};

class YTJS : public QObject {
    Q_OBJECT

public:
    static YTJS &instance();
    static Http &http();
    static Http &cachedHttp();

    explicit YTJS(QObject *parent = nullptr);
    bool checkError(const QJSValue &value);

    bool isInitialized();
    QQmlEngine &getEngine() { return *engine; }

signals:
    void initialized();
    void initFailed(QString message);

private:
    void initialize();
    QJSValue evaluate(const QString &js);

    // QQmlEngine gives us XMLHttpRequest, console, JSON
    QQmlEngine *engine;
    bool initializing = false;
    bool ready = false;
};

#endif // YTJS_H
