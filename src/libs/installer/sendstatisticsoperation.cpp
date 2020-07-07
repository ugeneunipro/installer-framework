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

#include "packagemanagercore.h"
#include "packagemanagerproxyfactory.h"
#include "sendstatisticsoperation.h"
#include "sendstatisticsoperation_p.h"

#include <QDebug>
#include <QThreadPool>

using namespace QInstaller;

SendStatisticsOperation::SendStatisticsOperation(PackageManagerCore *core)
    : UpdateOperation(core)
{
    setName(QLatin1String("SendStatistics"));
}

void SendStatisticsOperation::backup()
{
    // Do nothing
}

bool SendStatisticsOperation::performOperation()
{
    if (!checkArgumentCount(1, INT_MAX)) {
        qDebug() << "Failed to send statistics because of invalid arguments count: " << arguments().size();
        // Do not interrupt the installation
        return true;
    }

    const QStringList args = arguments();
    const QUrl url = args.at(0);
    QMap<QByteArray, QByteArray> additionalHeaders;
    for (int i = 1; i < args.size(); ++i) {
        QStringList pair = args.at(i).split(QLatin1String("="));
        if (2 != pair.size()) {
            qDebug() << "Failed to parse argument during statistics sending: " << args.at(i);
            continue;
        }

        additionalHeaders.insert(pair.first().toLatin1(), pair.last().toLatin1());
    }

    Receiver receiver;

    Runnable *runnable = new Runnable(url, additionalHeaders, packageManager()->proxyFactory());
    QObject::connect(runnable, &Runnable::finished, &receiver, &Receiver::runnableFinished/*, Qt::QueuedConnection*/);

    QEventLoop loop;
    QObject::connect(&receiver, &Receiver::finished, &loop, &QEventLoop::quit);
    if (QThreadPool::globalInstance()->tryStart(runnable)) {
        loop.exec();
    } else {
        // HACK: In case there is no availabe thread we should call it directly.
        runnable->run();
        receiver.runnableFinished(true, QString());
    }

    if (!receiver.success()) {
        // It's pity, but it's not critical
        qDebug() << "Failed to send statistics with the next arguments: " << args.join(QLatin1String(", "));
        qDebug() << "Because of error: " << receiver.errorString();
    }
    return true;
}

bool SendStatisticsOperation::undoOperation()
{
    return true;
}

bool SendStatisticsOperation::testOperation()
{
    return true;
}
