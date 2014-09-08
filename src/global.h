/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QStringList>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <cstdlib>
#include "networkaccess.h"
#include "diskcache.h"

namespace The {

    QHash<QString, QAction*>* globalActions() {
        static QHash<QString, QAction*> *actions = new QHash<QString, QAction*>;
        return actions;
    }

    QHash<QString, QMenu*>* globalMenus() {
        static QHash<QString, QMenu*> *menus = new QHash<QString, QMenu*>;
        return menus;
    }

    void maybeSetSystemProxy() {

        QNetworkProxyQuery proxyQuery(QUrl("http://www"));
        proxyQuery.setProtocolTag("http");
        QList<QNetworkProxy> proxylist = QNetworkProxyFactory::systemProxyForQuery(proxyQuery);

        for (int i = 0; i < proxylist.count(); i++) {
            QNetworkProxy proxy = proxylist.at(i);

            /*
            qDebug() << i << " type:"<< proxy.type();
            qDebug() << i << " host:" << proxy.hostName();
            qDebug() << i << " port:" << proxy.port();
            qDebug() << i << " user:" << proxy.user();
            qDebug() << i << " pass:" << proxy.password();
            */

            if (!proxy.hostName().isEmpty()) {
                qDebug() << "Using proxy:" << proxy.hostName() << proxy.port();
                QNetworkProxy::setApplicationProxy(proxy);
                return;
            }
        }
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

            // Remove trailing slash, if any
            // Fix by Eduardo Suarez-Santana
            http_proxy.remove(QRegExp("/$"));

            // parse username and password
            if (http_proxy.contains(QChar('@'))) {
                QStringList http_proxy_list = http_proxy.split(QChar('@'));
                QStringList http_proxy_user_pass = http_proxy_list[0].split(QChar(':'));
                if (http_proxy_user_pass.size() > 0) {
                    proxy_user = QUrl::fromPercentEncoding(http_proxy_user_pass[0].toUtf8());
                }
                if (http_proxy_user_pass.size() == 2) {
                    proxy_pass = QUrl::fromPercentEncoding(http_proxy_user_pass[1].toUtf8());
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

            /*
            qDebug() << "proxy_host: " << proxy_host;
            qDebug() << "proxy_port: " << proxy_port;
            qDebug() << "proxy_user: " << proxy_user;
            qDebug() << "proxy_pass: " << proxy_pass;
            */

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

                qDebug() << "Using HTTP proxy:" << http_proxy_env;
                QNetworkProxy::setApplicationProxy(proxy);
            }
        }
    }

    QNetworkAccessManager* networkAccessManager() {
        static QNetworkAccessManager *nam = 0;
        if (!nam) {
            networkHttpProxySetting();
            maybeSetSystemProxy();
            nam = new QNetworkAccessManager();
            QNetworkDiskCache *cache = new DiskCache();
#if QT_VERSION >= 0x050000
            QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/qt5";
#else
            QString cacheLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
            cache->setCacheDirectory(cacheLocation);
            nam->setCache(cache);
        }
        return nam;
    }

    NetworkAccess* http() {
        static NetworkAccess *na = new NetworkAccess();
        return na;
    }

}

#endif // GLOBAL_H
