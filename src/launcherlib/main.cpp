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

#define DECL_EXPORT extern "C" __attribute__ ((__visibility__("default")))

int  g_sigChldPipeFd = -1;
int  g_sigTermPipeFd = -1;
int  g_sigUsr1PipeFd = -1;

char g_dummyPipeData =  0;

static void sigChldHandler(int)
{
    write(g_sigChldPipeFd, &g_dummyPipeData, 1);
}

static void sigTermHandler(int)
{
    write(g_sigTermPipeFd, &g_dummyPipeData, 1);
}

static void sigUsr1Handler(int)
{
    write(g_sigUsr1PipeFd, &g_dummyPipeData, 1);
}

//! Main function
DECL_EXPORT int main(int argc, char * argv[])
{
    // Open the log
    Logger::openLog(PROG_NAME_LAUNCHER);
    Logger::logDebug("%s starting..", PROG_NAME_LAUNCHER);

    // Check that an instance of launcher is not already running
    if(!Daemon::lock())
    {
        Logger::logErrorAndDie(EXIT_FAILURE, "%s is already running \n", PROG_NAME_LAUNCHER);
    }

    // Create main daemon instance
    Daemon myDaemon(argc, argv);

    // Get fd's for signal pipes.
    g_sigChldPipeFd = myDaemon.sigChldPipeFd();
    g_sigTermPipeFd = myDaemon.sigTermPipeFd();
    g_sigUsr1PipeFd = myDaemon.sigUsr1PipeFd();

    // Install signal handlers
    signal(SIGCHLD, sigChldHandler); // reap zombies
    signal(SIGTERM, sigTermHandler); // exit launcher
    signal(SIGUSR1, sigUsr1Handler); // restore normal mode

    // Run the main loop
    myDaemon.run();

    // Close the log
    Logger::closeLog();

    return EXIT_SUCCESS;
}
