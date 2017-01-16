# A wrapper for the Qt Network Access API

This is just a wrapper around Qt's QNetworkAccessManager and friends. I use it in my Qt apps at http://flavio.tordini.org . It allows me to add missing functionality as needed, e.g.:

- Throttling (as required by many web APIs nowadays)
- Read timeouts (don't let your requests get stuck forever)
- Automatic retries
- User agent and request header defaults
- Partial requests
- Redirection support (now supported by Qt >= 5.6)

It has a simpler, higher-level API that I find easier to work with. The design emerged naturally in years of practical use.

A basic example:

```
QObject *reply = Http::instance().get("https://google.com/");
connect(reply, SIGNAL(data(QByteArray)), SLOT(onSuccess(QByteArray)));
connect(reply, SIGNAL(error(QString)), SLOT(onError(QString)));

void MyClass::onSuccess(const QByteArray &bytes) {
	qDebug() << "Feel the bytes!" << bytes;
}

void MyClass::onError(const QString &message) {
	qDebug() << "Something's wrong here" << message;
}
```

This is a real-world example of building a Http object suitable to a web service. It throttles requests, uses a custom user agent and caches results:

```
Http &myHttp() {
    static Http *http = [] {
        Http *http = new Http;
        http->addRequestHeader("User-Agent", userAgent());

        ThrottledHttp *throttledHttp = new ThrottledHttp(*http);
        throttledHttp->setMilliseconds(1000);

        CachedHttp *cachedHttp = new CachedHttp(*throttledHttp, "mycache");
        cachedHttp->setMaxSeconds(86400 * 30);

        return cachedHttp;
    }();
    return *http;
}
```

You can use this library under the MIT license and at your own risk. If you do, you're welcome contributing your changes and fixes.

Cheers,

Flavio
