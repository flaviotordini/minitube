#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtGui>
#include <QStringList>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <cstdlib>
#include "networkaccess.h"

namespace The {

    static QMap<QString, QAction*> *g_actions = 0;

    QMap<QString, QAction*>* globalActions() {
        if (!g_actions)
            g_actions = new QMap<QString, QAction*>;
        return g_actions;
    }

    static QMap<QString, QMenu*> *g_menus = 0;

    QMap<QString, QMenu*>* globalMenus() {
        if (!g_menus)
            g_menus = new QMap<QString, QMenu*>;
        return g_menus;
    }

    void networkHttpProxySetting() {
        char *http_proxy_env;
        http_proxy_env = std::getenv("http_proxy");
        if (!http_proxy_env) {
            http_proxy_env = std::getenv("HTTP_PROXY");
        }

        if (http_proxy_env) {
            QString proxy_host = "";
            QString proxy_port = "";
            QString proxy_user = "";
            QString proxy_pass = "";
            QString http_proxy = QString(http_proxy_env);
            http_proxy.remove(QRegExp("^http://"));

            // parse username and password
            if (http_proxy.contains(QChar('@'))) {
                QStringList http_proxy_list = http_proxy.split(QChar('@'));
                QStringList http_proxy_user_pass = http_proxy_list[0].split(QChar(':'));
                if (http_proxy_user_pass.size() > 0) {
                    proxy_user = http_proxy_user_pass[0];
                }
                if (http_proxy_user_pass.size() == 2) {
                    proxy_pass = http_proxy_user_pass[1];
                }
                if (http_proxy_list.size() > 1) {
                    http_proxy = http_proxy_list[1];
                }
            }

            // parse hostname and port
            QStringList http_proxy_list = http_proxy.split(QChar(':'));
            if (http_proxy_list.size() > 0) {
                proxy_host = http_proxy_list[0];
            }
            if (http_proxy_list.size() > 1) {
                proxy_port = http_proxy_list[1];
            }

            qDebug() << "proxy_host: " << proxy_host;
            qDebug() << "proxy_port: " << proxy_port;
            qDebug() << "proxy_user: " << proxy_user;
            qDebug() << "proxy_pass: " << proxy_pass;

            // set proxy setting
            if (!proxy_host.isEmpty()) {
                QNetworkProxy proxy;
                proxy.setType(QNetworkProxy::HttpProxy);
                proxy.setHostName(proxy_host);
                if (!proxy_port.isEmpty()) {
                    proxy.setPort(proxy_port.toUShort());
                }
                if (!proxy_user.isEmpty()) {
                    proxy.setUser(proxy_user);
                }
                if (!proxy_pass.isEmpty()) {
                    proxy.setPassword(proxy_pass);
                }
                QNetworkProxy::setApplicationProxy(proxy);
            }
        }
    }

    static QNetworkAccessManager *nam = 0;

    QNetworkAccessManager* networkAccessManager() {
        if (!nam) {
            networkHttpProxySetting();
            nam = new QNetworkAccessManager();

            // A simple disk based cache
            /*
            QNetworkDiskCache *cache = new QNetworkDiskCache();
            QString cacheLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
            qDebug() << cacheLocation;
            cache->setCacheDirectory(cacheLocation);
            nam->setCache(cache);
            */
        }
        return nam;
    }

    static NetworkAccess *g_http = 0;
    NetworkAccess* http() {
        if (!g_http)
            g_http = new NetworkAccess();
        return g_http;
    }

}

#endif // GLOBAL_H

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
