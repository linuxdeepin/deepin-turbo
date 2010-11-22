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

#include "daemon.h"
#include "logger.h"

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <fcntl.h>
#include <sys/file.h>

#include <QtCore/QtGlobal>

//! Signal handler to reap zombie processes
void reapZombies(int)
{
    if (Daemon::instance())
    {
        Daemon::instance()->reapZombies();
    }
}

//! Signal handler to kill booster
void exitBooster(int)
{
    Logger::logErrorAndDie(EXIT_FAILURE, "due to parent process applauncherd died, booster exit too \n");
}

void exitLauncher(int)
{
    exit(0);
}

//! Main function
Q_DECL_EXPORT int main(int argc, char * argv[])
{

    // Open the log
    Logger::openLog(PROG_NAME_LAUNCHER);
    Logger::logNotice("%s starting..", PROG_NAME_LAUNCHER);

    // Check that an instance of launcher is not already running
    if(!Daemon::lock())
    {
        Logger::logErrorAndDie(EXIT_FAILURE, "%s is already running \n", PROG_NAME_LAUNCHER);
    }

    // Install signal handlers
    signal(SIGCHLD, reapZombies);
    signal(SIGHUP,  exitBooster);
    signal(SIGTERM, exitLauncher);

    // Create main daemon instance
    Daemon myDaemon(argc, argv);

    // Run the main loop
    myDaemon.run();

    // Close the log
    Logger::closeLog();

    return EXIT_SUCCESS;
}

