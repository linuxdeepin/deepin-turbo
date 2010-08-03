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

//! Signal handler to reap zombie processes
void reapZombies(int)
{
    if (Daemon::instance())
        Daemon::instance()->reapZombies();
}

//! Signal handler to kill booster
void exitBooster(int)
{
    Logger::logErrorAndDie(EXIT_FAILURE, "due to parent process applauncherd died, booster exit too");
}

void exitLauncher(int)
{
    exit(0);
}

void usage()
{
    std::cout << "Usage: "<< PROG_NAME << " [options]\n"
              << "\n"
              << "Options:\n"
              << "  --daemon            Fork and go into the background.\n"
              //<< "  --pidfile FILE      Specify a different pid file (default " << LAUNCHER_PIDFILE << " ).\n"
              //<< "  --send-app-died     Send application died signal.\n"
              << "  --quiet             Do not print anything.\n"
              << "  --help              Print this help message.\n"
              << "\n"
              << "Use the invoker to start a <shared object> from the launcher.\n"
              << "Where <shared object> is a binary including a 'main' symbol.\n"
              << "Note that the binary needs to be linked with -shared or -pie.\n";

    exit(EXIT_SUCCESS);
}

//! Main function
int main(int argc, char * argv[])
{
    // Print usage if requested on commandline
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--help")
        {
            usage();
        }
    }

    // Open the log
    Logger::openLog(PROG_NAME);
    Logger::logNotice("%s starting..", PROG_NAME);

    // Check that an instance of launcher is not already running
    if(!Daemon::lock())
    {
        Logger::logErrorAndDie(EXIT_FAILURE, "try to launch second instance");
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

