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

#include <QtConcurrentRun>
#include <MApplication>
#include <sys/socket.h>

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

const string MBooster::m_socketId  = "/tmp/boostm";
const string MBooster::m_temporaryProcessName = "booster-m";
int MBooster::m_ProcessID = 0;
int MBooster::m_sighupFd[2];
struct sigaction MBooster::m_oldSigAction;

MBooster::MBooster() : m_item(0)
{
    // Install signals handler e.g. to exit cleanly if launcher dies.
    // This is a problem because MBooster runs a Qt event loop.
    setupUnixSignalHandlers();

    // Create socket pair for SIGTERM
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sighupFd))
    {
       Logger::logError("Couldn't create HUP socketpair");
    }
    else
    {
        // Install a socket notifier on the socket
        m_snHup.reset(new QSocketNotifier(m_sighupFd[1], QSocketNotifier::Read, this));
        connect(m_snHup.get(), SIGNAL(activated(int)), this, SLOT(handleSigHup()));
    }
}

//
// All this signal handling code is taken from Qt's Best Practices:
// http://doc.qt.nokia.com/latest/unix-signals.html
//

void MBooster::hupSignalHandler(int)
{
    char a = 1;
    ::write(m_sighupFd[0], &a, sizeof(a));
}

void MBooster::handleSigHup()
{
    ::_exit(EXIT_SUCCESS);
}

bool MBooster::setupUnixSignalHandlers()
{
    struct sigaction hup;

    hup.sa_handler = MBooster::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, &m_oldSigAction) > 0)
    {
        return false;
    }

    return true;
}

bool MBooster::restoreUnixSignalHandlers()
{
    if (sigaction(SIGHUP, &m_oldSigAction, 0) > 0)
    {
        return false;
    }

    return true;
}

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

void MBooster::setProcessId(int pid)
{
    m_ProcessID = pid;
}

int MBooster::processId()
{
    return m_ProcessID;
}

bool MBooster::receiveDataFromInvoker()
{
    // Setup the conversation channel with the invoker.
    setConnection(new Connection(socketId()));

    // exit from event loop when invoker is ready to connect
    connect(this, SIGNAL(connectionAccepted()), MApplication::instance() , SLOT(quit()));

    // enable theme change handler
    m_item = new MGConfItem(MEEGOTOUCH_THEME_GCONF_KEY, 0);
    connect(m_item, SIGNAL(valueChanged()), this, SLOT(notifyThemeChange()));

    // start another thread to listen connection from invoker
    QtConcurrent::run(this, &MBooster::accept);

    // Run event loop so MApplication and MApplicationWindow objects can receive notifications
    MApplication::exec();

    // disable theme change handler
    disconnect(m_item, 0, this, 0);
    delete m_item;
    m_item = NULL;

    // Restore signal handlers to previous values
    restoreUnixSignalHandlers();

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

void MBooster::notifyThemeChange()
{
    MApplication::quit();
    ::_exit(EXIT_SUCCESS);
}

void MBooster::accept()
{
    if (connection()->accept(appData()))
    {
        emit connectionAccepted();
    }
}
