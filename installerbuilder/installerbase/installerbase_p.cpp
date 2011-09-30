/**************************************************************************
**
** This file is part of Qt SDK**
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
**
** Contact:  Nokia Corporation qt-info@nokia.com**
**
** No Commercial Usage
**
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception version
** 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you are unsure which license is appropriate for your use, please contact
** (qt-info@nokia.com).
**
**************************************************************************/
#include "installerbase_p.h"

#include <QtCore/QDebug>
#include <QtGui/QMessageBox>


// -- MyCoreApplication

MyCoreApplication::MyCoreApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv)
{
}

// reimplemented from QCoreApplication so we can throw exceptions in scripts and slots
bool MyCoreApplication::notify(QObject *receiver, QEvent *event)
{
    try {
        return QCoreApplication::notify(receiver, event);
    } catch(std::exception &e) {
        qCritical() << "Exception thrown:" << e.what();
    }
    return false;
}


// -- MyApplication

MyApplication::MyApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

// reimplemented from QApplication so we can throw exceptions in scripts and slots
bool MyApplication::notify(QObject *receiver, QEvent *event)
{
    try {
        return QApplication::notify(receiver, event);
    } catch(std::exception &e) {
        qCritical() << "Exception thrown:" << e.what();
    }
    return false;
}