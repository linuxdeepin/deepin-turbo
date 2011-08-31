/***************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "qdeclarativebooster.h"
#include "mdeclarativecache.h"
#include "connection.h"

const string QDeclarativeBooster::m_socketId = "/tmp/boostd";
const string QDeclarativeBooster::m_temporaryProcessName = "booster-d";

const string & QDeclarativeBooster::socketId() const
{
    return m_socketId;
}

const string & QDeclarativeBooster::socketName()
{
    return m_socketId;
}

const string & QDeclarativeBooster::temporaryProcessName()
{
    return m_temporaryProcessName;
}

const string & QDeclarativeBooster::boosterTemporaryProcessName() const
{
    return temporaryProcessName();
}

char QDeclarativeBooster::type()
{
    return 'd';
}

bool QDeclarativeBooster::preload()
{
    MDeclarativeCache::populate();
    return true;
}

bool QDeclarativeBooster::receiveDataFromInvoker(int socketFd)
{
    // Use the default implementation if in boot mode
    // (it won't require QApplication running).

    if (bootMode())
    {
        return Booster::receiveDataFromInvoker(socketFd);
    }
    else
    {
        // Setup the conversation channel with the invoker.
        setConnection(new Connection(socketFd));

        EventHandler handler(this, EventHandler::QEventHandler);
        handler.runEventLoop();

        if (!connection()->connected())
        {
            return false;
        }

        // Receive application data from the invoker
        if(!connection()->receiveApplicationData(appData()))
        {
            connection()->close();
            return false;
        }

        // Close the connection if exit status doesn't need
        // to be sent back to invoker
        if (!connection()->isReportAppExitStatusNeeded())
        {
            connection()->close();
        }

        return true;
    }
}


void QDeclarativeBooster::preinit()
{
    QString appName = QFileInfo(m_appData->argv()[0]).fileName();

    QString appClass = appName.left(1).toUpper();
    if (appName.length() > 1)
        appClass += appName.right(appName.length() - 1);

    char* app_name = qstrdup(appName.toLatin1().data());
    QApplication::setAppName(app_name);

    char* app_class = qstrdup(appClass.toLatin1().data());
    QApplication::setAppClass(app_class);
}
