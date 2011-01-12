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

#include "wrtbooster.h"
#include "logger.h"
#include "connection.h"

#include <QtConcurrentRun>
#include <MApplication>
#include <sys/socket.h>

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

#ifdef HAVE_WRT
#include <wrtcomponentcache.h>
#endif

const string WRTBooster::m_socketId  = "/tmp/boostw";
const string WRTBooster::m_temporaryProcessName = "booster-w";

WRTBooster::WRTBooster()
{}

//
// All this signal handling code is taken from Qt's Best Practices:
// http://doc.qt.nokia.com/latest/unix-signals.html
//


const string & WRTBooster::socketId() const
{
    return m_socketId;
}

bool WRTBooster::preload()
{
#ifdef HAVE_MCOMPONENTCACHE
    MComponentCache::populateForWRTApplication();
#endif

#ifdef HAVE_WRT
    WrtComponentCache::populateCache();
#endif
    return true;
}

const string & WRTBooster::socketName()
{
    return m_socketId;
}

const string & WRTBooster::temporaryProcessName()
{
    return m_temporaryProcessName;
}

const string & WRTBooster::boosterTemporaryProcessName() const
{
    return temporaryProcessName();
}

char WRTBooster::type()
{
    return 'w';
}

bool WRTBooster::receiveDataFromInvoker(int socketFd)
{
    // Setup the conversation channel with the invoker.
    setConnection(new Connection(socketFd));

    EventHandler handler(this);
    handler.runEventLoop();

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

