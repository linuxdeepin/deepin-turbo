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
int WRTBooster::m_sighupFd[2];
struct sigaction WRTBooster::m_oldSigAction;

WRTBooster::WRTBooster() : m_item(0)
{}

//
// All this signal handling code is taken from Qt's Best Practices:
// http://doc.qt.nokia.com/latest/unix-signals.html
//

void WRTBooster::hupSignalHandler(int)
{
    char a = 1;
    ::write(m_sighupFd[0], &a, sizeof(a));
}

void WRTBooster::handleSigHup()
{
    MApplication::quit();
    ::_exit(EXIT_SUCCESS);
}

bool WRTBooster::setupUnixSignalHandlers()
{
    struct sigaction hup;

    hup.sa_handler = WRTBooster::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, &m_oldSigAction) > 0)
    {
        return false;
    }

    return true;
}

bool WRTBooster::restoreUnixSignalHandlers()
{
    if (sigaction(SIGHUP, &m_oldSigAction, 0) > 0)
    {
        return false;
    }

    return true;
}

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

    // Exit from event loop when invoker is ready to connect
    connect(this, SIGNAL(connectionAccepted()), MApplication::instance() , SLOT(quit()));

    // Enable theme change handler
    m_item = new MGConfItem(MEEGOTOUCH_THEME_GCONF_KEY, 0);
    connect(m_item, SIGNAL(valueChanged()), this, SLOT(notifyThemeChange()));

    // Start another thread to listen connection from invoker
    QtConcurrent::run(this, &WRTBooster::accept);

    // Create socket pair for SIGHUP
    bool handlerIsSet = false;
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sighupFd))
    {
        Logger::logError("WRTBooster: Couldn't create HUP socketpair");
    }
    else
    {
        // Install signal handler e.g. to exit cleanly if launcher dies.
        // This is a problem because MBooster runs a Qt event loop.
        setupUnixSignalHandlers();

        // Install a socket notifier on the socket
        connect(new QSocketNotifier(m_sighupFd[1], QSocketNotifier::Read, this),
                SIGNAL(activated(int)), this, SLOT(handleSigHup()));

        handlerIsSet = true;
    }

    // Run event loop so MApplication and MApplicationWindow objects can receive notifications
    MApplication::exec();

    // Disable theme change handler
    disconnect(m_item, 0, this, 0);
    delete m_item;
    m_item = NULL;

    // Restore signal handlers to previous values
    if (handlerIsSet)
    {
        restoreUnixSignalHandlers();
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

void WRTBooster::notifyThemeChange()
{
    MApplication::quit();
    ::_exit(EXIT_SUCCESS);
}

void WRTBooster::accept()
{
    if (connection()->accept(appData()))
    {
        emit connectionAccepted();
    }
}
