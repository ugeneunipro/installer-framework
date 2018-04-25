/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef SENDSTATISTICSOPERATION_P_H
#define SENDSTATISTICSOPERATION_P_H

#include "sendstatisticsoperation.h"

#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRunnable>
#include <QThread>
#include <QTimer>

namespace QInstaller {

class SendStatisticsOperation::Runnable : public QObject, public QRunnable
{
    Q_OBJECT
    Q_DISABLE_COPY(Runnable)

public:
    Runnable(const QUrl &url, const QMap<QByteArray, QByteArray> &additionalHeaders, QNetworkProxyFactory *proxyFactory)
        : m_url(url)
        , m_additionalHeaders(additionalHeaders)
        , m_proxyFactory(proxyFactory)
        , http(NULL)
    {
    }

    void run()
    {
        QNetworkAccessManager manager;
        QEventLoop eventLoop;
        connect(this, &Runnable::finished, &eventLoop, &QEventLoop::quit);

#ifndef QT_NO_SSL
        connect(&manager, &QNetworkAccessManager::sslErrors,
                this, &Runnable::onSslErrors);
#endif
        connect(&manager, &QNetworkAccessManager::authenticationRequired,
                this, &Runnable::onAuthenticationRequired);
        connect(&manager, &QNetworkAccessManager::networkAccessibleChanged,
                this, &Runnable::onNetworkAccessibleChanged);

        manager.setProxyFactory(m_proxyFactory);

        http = manager.get(prepareRequest());
        connect(http, SIGNAL(finished()), this, SLOT(httpReqFinished()));

        QTimer::singleShot(TIMEOUT, this, &Runnable::timeout);
        eventLoop.exec();
    }

signals:
    void finished(bool success, const QString &errorString);

private slots:
    void onSslErrors(QNetworkReply *, const QList<QSslError> &errors) {
        QString errorString = tr("The following SSL errors encountered:");
        foreach (const QSslError &error, errors) {
            errorString += QLatin1Char('\n') + error.errorString();
        }
        emit finished(false, errorString);
    }

    void onAuthenticationRequired() {
        emit finished(false, tr("The server asks for an authentification, do not bother the user with it"));
    }

    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
        if (QNetworkAccessManager::NotAccessible == accessible) {
            emit finished(false, tr("The network becomes not accessable, cancel the operation"));
        }
    }

    void httpReqFinished() {
        if (QNetworkReply::NoError == http->error()) {
            emit finished(true, QLatin1String());
        } else {
            emit finished(false, http->errorString());
        }
    }

    void timeout() {
        http->abort();
        emit finished(false, tr("Operation cancelled: timeout"));
    }

private:
    QNetworkRequest prepareRequest() const
    {
        QNetworkRequest request(m_url);
        QMapIterator<QByteArray, QByteArray> it(m_additionalHeaders);
        while (it.hasNext()) {
            request.setRawHeader(it.key(), it.value());
        }

        return request;
    }

    QUrl m_url;
    QMap<QByteArray, QByteArray> m_additionalHeaders;
    QNetworkProxyFactory *m_proxyFactory;
    QNetworkReply *http;

    static const int TIMEOUT = 5000;
};

class SendStatisticsOperation::Receiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Receiver)

public:
    Receiver() = default;

    bool success() const {
        return m_success;
    }

    QString errorString() const {
        return m_errorString;
    }

public slots:
    void runnableFinished(bool ok, const QString &msg)
    {
        m_success = ok;
        m_errorString = msg;
        emit finished();
    }

signals:
    void finished();

private:
    bool m_success = false;
    QString m_errorString;
};

}

#endif // SENDSTATISTICSOPERATION_P_H
