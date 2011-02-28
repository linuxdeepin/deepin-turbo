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

int  g_sigPipeFd       = -1;
char g_pipeDataSigChld = SIGCHLD;
char g_pipeDataSigTerm = SIGTERM;
char g_pipeDataSigUsr1 = SIGUSR1;
char g_pipeDataSigUsr2 = SIGUSR2;

static void sigChldHandler(int)
{
    write(g_sigPipeFd, &g_pipeDataSigChld, 1);
}

static void sigTermHandler(int)
{
    write(g_sigPipeFd, &g_pipeDataSigTerm, 1);
}

static void sigUsr1Handler(int)
{
    write(g_sigPipeFd, &g_pipeDataSigUsr1, 1);
}

static void sigUsr2Handler(int)
{
    write(g_sigPipeFd, &g_pipeDataSigUsr2, 1);
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

    // Get fd for signal pipe.
    g_sigPipeFd = myDaemon.sigPipeFd();

    // Install signal handlers
    signal(SIGCHLD, sigChldHandler); // reap zombies
    signal(SIGTERM, sigTermHandler); // exit launcher
    signal(SIGUSR1, sigUsr1Handler); // enter normal mode from boot mode
    signal(SIGUSR2, sigUsr2Handler); // enter boot mode (same as --boot-mode)

    // Run the main loop
    myDaemon.run();

    // Close the log
    Logger::closeLog();

    return EXIT_SUCCESS;
}
