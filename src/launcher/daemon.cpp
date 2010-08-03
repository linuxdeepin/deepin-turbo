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
#include "connection.h"
#include "booster.h"
#include "mbooster.h"
#include "qtbooster.h"

#include <cstdlib>
#include <cerrno>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include <signal.h>

#include <fcntl.h>
#include <iostream>

Daemon * Daemon::m_instance = NULL;
int Daemon::m_lockFd = -1;

Daemon::Daemon(int & argc, char * argv[]) :
    m_daemon(false),
    m_quiet(false)
{
    if (!Daemon::m_instance)
    {
        Daemon::m_instance = this;
    }
    else
    {
        std::cerr << "Daemon already created!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Parse arguments
    parseArgs(ArgVect(argv, argv + argc));

    // Disable console output
    if (m_quiet)
        consoleQuiet();

    // Store arguments list
    m_initialArgv = argv;
    m_initialArgc = argc;

    // Daemonize if desired
    if (m_daemon)
    {
        daemonize();
    }
}

void Daemon::consoleQuiet()
{
    close(0);
    close(1);
    close(2);

    if (open("/dev/null", O_RDONLY) < 0)
        Logger::logErrorAndDie(EXIT_FAILURE, "opening /dev/null readonly");

    int fd = open("/dev/null", O_WRONLY);
    if ((fd == -1) || (dup(fd) < 0))
        Logger::logErrorAndDie(EXIT_FAILURE, "opening /dev/null writeonly");
}

Daemon * Daemon::instance()
{
    return Daemon::m_instance;
}

Daemon::~Daemon()
{}

bool Daemon::lock()
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 1;

    if((m_lockFd = open("/tmp/applauncherd.lock", O_WRONLY | O_CREAT, 0666)) == -1)
        return false;

    if(fcntl(m_lockFd, F_SETLK, &fl) == -1)
        return false;

    return true;
}

void Daemon::unlock()
{
    if (m_lockFd != -1)
    {
        close(m_lockFd);
        m_lockFd = -1;
    }
}

void Daemon::run()
{
    // Make sure that LD_BIND_NOW does not prevent dynamic linker to
    // use lazy binding in later dlopen() calls.
    unsetenv("LD_BIND_NOW");

    // create sockets for each of the boosters
    Connection::initSocket(MBooster::socketName());
    Connection::initSocket(QtBooster::socketName());

    // Pipe used to tell the parent that a new
    // booster is needed
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        Logger::logErrorAndDie(EXIT_FAILURE, "Creating a pipe failed!!!\n");
    }

    forkBooster(MBooster::type(), pipefd);
    forkBooster(QtBooster::type(),  pipefd);

    while (true)
    {
        // Wait for something appearing in the pipe
        char msg;
        ssize_t count = read(pipefd[0], reinterpret_cast<void *>(&msg), 1);
        if (count)
        {
            // Guarantee some time for the just launched application to
            // start up before forking new booster. Not doing this would
            // slow down the start-up significantly on single core CPUs.
            sleep(2);

            // Fork a new booster of the given type
            forkBooster(msg, pipefd);
        }
        else
        {
            Logger::logWarning("Nothing read from the pipe\n");
        }
    }
}

bool Daemon::forkBooster(char type, int pipefd[2])
{
    // Fork a new process
    pid_t newPid = fork();

    if (newPid == -1)
        Logger::logErrorAndDie(EXIT_FAILURE, "Forking while invoking");

    if (newPid == 0) /* Child process */
    {
        // Reset used signal handlers
        signal(SIGCHLD, SIG_DFL);

        // Will get this signal if applauncherd dies
        prctl(PR_SET_PDEATHSIG, SIGHUP);

        // Close unused read end
        close(pipefd[0]);

        // close lock file, it's not needed in the booster
        Daemon::unlock();

        if (setsid() < 0)
        {
            Logger::logError("Setting session id\n");
        }

        Logger::logNotice("Running a new Booster of %c type...", type);

        // Create a new booster and initialize it
        Booster * booster = NULL;
        if (MBooster::type() == type)
        {
            booster = new MBooster();
        }
        else if (QtBooster::type() == type)
        {
            booster = new QtBooster();
        }
        else
        {
            Logger::logErrorAndDie(EXIT_FAILURE, "Unknown booster type \n");
        }

        // Drop priority (nice = 10)
        booster->pushPriority(10);

        // Preload stuff
        booster->preload();

        // Clean-up all the env variables
        clearenv();

        // Rename launcher process to booster
        booster->renameProcess(m_initialArgc, m_initialArgv);

        // Restore priority
        booster->popPriority();

        // Wait and read commands from the invoker
        Logger::logNotice("Wait for message from invoker");
        booster->readCommand();

        // Give to the process an application specific name
        booster->renameProcess(m_initialArgc, m_initialArgv);

        // Signal the parent process that it can create a new
        // waiting booster process and close write end
        const char msg = booster->boosterType();
        ssize_t ret = write(pipefd[1], reinterpret_cast<const void *>(&msg), 1);
        if (ret == -1) {
            Logger::logError("Can't send signal to launcher process' \n");
        }

        close(pipefd[1]);

        // Don't care about fate of parent applauncherd process any more
        prctl(PR_SET_PDEATHSIG, 0);

        // Set dumpable flag
        prctl(PR_SET_DUMPABLE, 1);
        
        // Run the current Booster
        booster->run();

        // Finish
        delete booster;
        exit(EXIT_SUCCESS);
    }
    else /* Parent process */
    {
        // Store the pid so that we can reap it later
        m_children.push_back(newPid);
    }

    return true;
}

void Daemon::reapZombies()
{
    PidVect::iterator i(m_children.begin());
    while (i != m_children.end())
    {
        if (waitpid(*i, NULL, WNOHANG))
        {
            i = m_children.erase(i);
        }
        else
        {
            i++;
        }
    }
}

void Daemon::daemonize()
{
    // Our process ID and Session ID
    pid_t pid, sid;

    // Fork off the parent process: first fork
    pid = fork();
    if (pid < 0)
    {
        Logger::logError("Unable to fork daemon, code %d (%s)", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    // Fork off the parent process: second fork
    pid = fork();
    if (pid < 0)
    {
        Logger::logError("Unable to fork daemon, code %d (%s)", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    // Change the file mode mask
    umask(0);

    // Open any logs here

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0)
    {
        Logger::logError("Unable to create a new session, code %d (%s)", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Change the current working directory
    if ((chdir("/")) < 0)
    {
        Logger::logError("Unable to change directory to %s, code %d (%s)", "/", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Open file descriptors pointing to /dev/null
    // Redirect standard file descriptors to /dev/null
    // Close new file descriptors

    const int new_stdin  = open("/dev/null", O_RDONLY);
    if (new_stdin != -1) {
        dup2(new_stdin,  STDIN_FILENO);
        close(new_stdin);
    }

    const int new_stdout = open("/dev/null", O_WRONLY);
    if (new_stdout != -1) {
        dup2(new_stdout, STDOUT_FILENO);
        close(new_stdout);
    }

    const int new_stderr = open("/dev/null", O_WRONLY);
    if (new_stderr != -1) {
        dup2(new_stderr, STDERR_FILENO);
        close(new_stderr);
    }
}

void Daemon::parseArgs(const ArgVect & args)
{
    for (ArgVect::const_iterator i(args.begin()); i != args.end(); i++)
    {
        if ((*i) == "--daemon")
        {
            m_daemon = true;
        }
        else if  ((*i) ==  "--quiet")
        {
            m_quiet = true;
        }
    }
}
