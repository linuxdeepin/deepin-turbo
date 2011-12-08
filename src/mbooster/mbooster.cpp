/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "mbooster.h"
#include "logger.h"
#include "connection.h"

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

const string MBooster::m_socketId  = "/tmp/boostm";
const string MBooster::m_temporaryProcessName = "booster-m";

const string & MBooster::socketId() const
{
    return m_socketId;
}

bool MBooster::preload()
{
#ifdef HAVE_MCOMPONENTCACHE
    // Populate the cache (instantiates an MApplicationWindow and
    // an MApplication)
    MComponentCache::populateForMApplication();
#endif

    return true;
}

const string & MBooster::socketName()
{
    return m_socketId;
}

const string & MBooster::temporaryProcessName()
{
    return m_temporaryProcessName;
}

const string & MBooster::boosterTemporaryProcessName() const
{
    return temporaryProcessName();
}

char MBooster::type()
{
    return 'm';
}

bool MBooster::receiveDataFromInvoker(int socketFd)
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

        EventHandler handler(this, EventHandler::MEventHandler);
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

void MBooster::preinit()
{
    QString appName = QFileInfo(m_appData->argv()[0]).fileName();

    QString appClass = appName.left(1).toUpper();
    if (appName.length() > 1)
        appClass += appName.right(appName.length() - 1);

    char* app_name = qstrdup(appName.toLatin1().data());
    QApplication::setAppName(app_name);

    char* app_class = qstrdup(appClass.toLatin1().data());
    QApplication::setAppClass(app_class);

    // Set the magic attribute so that paths are reinitialized
    QApplication::setAttribute(Qt::AA_LinuxReinitPathsFromArgv0, true);

}
