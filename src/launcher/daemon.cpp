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
#include "wrtbooster.h"
#include "monitorbooster.h"
#include "boosterfactory.h"
#include "preload.h"


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
#include <dlfcn.h>

Daemon * Daemon::m_instance = NULL;
int Daemon::m_lockFd = -1;
const int Daemon::m_boosterSleepTime = 2;

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
        Logger::logErrorAndDie(EXIT_FAILURE, "Daemon: Daemon already created!\n");
    }

    // Parse arguments
    parseArgs(ArgVect(argv, argv + argc));

    // Disable console output
    if (m_quiet)
        consoleQuiet();

    // Store arguments list
    m_initialArgv = argv;
    m_initialArgc = argc;

    if (pipe(m_pipefd) == -1)
    {
        Logger::logErrorAndDie(EXIT_FAILURE, "Daemon: Creating a pipe failed!!!\n");
    }

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
        Logger::logErrorAndDie(EXIT_FAILURE, "Daemon: opening /dev/null readonly");

    int fd = open("/dev/null", O_WRONLY);
    if ((fd == -1) || (dup(fd) < 0))
        Logger::logErrorAndDie(EXIT_FAILURE, "Daemon: opening /dev/null writeonly");
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

void Daemon::preload()
{
    vector<string> vLibs(libs, libs + sizeof(libs) / sizeof(char *));
    for (size_t i = 0; i < vLibs.size(); i++)
    {
        void* handle = dlopen(vLibs[i].c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle)
        {
            Logger::logError("Daemon: Can't load %s library\n", vLibs[i].c_str());
        }
    }
}

void Daemon::run()
{
    // Make sure that LD_BIND_NOW does not prevent dynamic linker to
    // use lazy binding in later dlopen() calls.
    unsetenv("LD_BIND_NOW");

    // load and resolve all undefined symbols for each dynamic library from the list 
    preload();

    // Create sockets for each of the boosters
    Connection::initSocket(MBooster::socketName());
    Connection::initSocket(QtBooster::socketName());
    Connection::initSocket(WRTBooster::socketName());

    // Fork each booster for the first time
    forkBooster(MBooster::type());
    forkBooster(QtBooster::type());
    forkBooster(WRTBooster::type());
    forkBooster(MonitorBooster::type());

    while (true)
    {
        // Wait for something appearing in the pipe
        char msg;
        ssize_t count = read(m_pipefd[0], reinterpret_cast<void *>(&msg), 1);
        if (count)
        {
            // read pid of peer invoker
            pid_t invoker_pid;
            count = read(m_pipefd[0], reinterpret_cast<void *>(&invoker_pid), sizeof(pid_t));

            if (count < static_cast<ssize_t>(sizeof(pid_t)))
            {
                Logger::logErrorAndDie(EXIT_FAILURE, "Daemon: pipe connection with booster failed");
            }
            else
            {
                Logger::logInfo("Daemon: invoker's pid: %d \n", invoker_pid);
            }

            if (invoker_pid != 0)
            {
                // store booster - invoker pids pair
                pid_t booster_pid = BoosterFactory::getBoosterPidForType(msg);
                m_kindergarten[booster_pid] = invoker_pid;
            }

            // Fork a new booster of the given type

            // 2nd param guarantees some time for the just launched application
            // to start up before forking new booster. Not doing this would
            // slow down the start-up significantly on single core CPUs.

            forkBooster(msg, m_boosterSleepTime);
        }
        else
        {
            Logger::logWarning("Daemon: Nothing read from the pipe\n");
        }
    }
}


void Daemon::forkBooster(char type, int sleepTime)
{
    // Fork a new process
    pid_t newPid = fork();

    if (newPid == -1)
        Logger::logErrorAndDie(EXIT_FAILURE, "Daemon: Forking while invoking");

    if (newPid == 0) /* Child process */
    {
        // Reset used signal handlers
        signal(SIGCHLD, SIG_DFL);

        // Will get this signal if applauncherd dies
        prctl(PR_SET_PDEATHSIG, SIGHUP);

        // Close unused read end
        close(m_pipefd[0]);

        // close lock file, it's not needed in the booster
        Daemon::unlock();

        if (setsid() < 0)
        {
            Logger::logError("Daemon: Setting session id\n");
        }

        // Guarantee some time for the just launched application to
        // start up before forking new booster if needed.
        if (sleepTime)
            sleep(sleepTime);

        Logger::logNotice("Daemon: Running a new Booster of %c type...", type);

        // Create a new booster and initialize it
        Booster * booster = BoosterFactory::create(type);
        if (booster)
        {
            initializeBooster(booster);
        }
        else
        {
            Logger::logErrorAndDie(EXIT_FAILURE, "Daemon: Unknown booster type \n");
        }

        // close pipe
        close(m_pipefd[1]);

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

        // Set current process ID globally to the given booster type
        // so that we now which booster to restart if on exits
        BoosterFactory::setProcessIdToBooster(type, newPid);
    }
}

void Daemon::initializeBooster(Booster * booster)
{
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
    Logger::logNotice("Daemon: Wait for message from invoker");
    booster->readCommand();

    // Give to the process an application specific name
    booster->renameProcess(m_initialArgc, m_initialArgv);

    // Signal the parent process that it can create a new
    // waiting booster process and close write end
    const char msg = booster->boosterType();
    ssize_t ret = write(m_pipefd[1], reinterpret_cast<const void *>(&msg), 1);
    if (ret == -1) {
        Logger::logError("Daemon: Can't send signal to launcher process' \n");
    }

    // Send to the parent process pid of invoker for tracking
    pid_t pid = booster->invokersPid();
    ret = write(m_pipefd[1], reinterpret_cast<const void *>(&pid), sizeof(pid_t));
    if (ret == -1) {
        Logger::logError("Daemon: Can't send invoker's pid to launcher process' \n");
    }
}

void Daemon::reapZombies()
{
    PidVect::iterator i(m_children.begin());
    while (i != m_children.end())
    {
        int status;
        pid_t pid = waitpid(*i, &status, WNOHANG);
        if (pid)
        {
            i = m_children.erase(i);

            PidMap::iterator it = m_kindergarten.find(pid);
            if (it != m_kindergarten.end())
            {
                Logger::logInfo("Daemon: terminated process is in the kendergarten");

                if (WIFSIGNALED(status))
                {
                    int signal = WTERMSIG(status);
                    pid_t invoker_pid = (*it).second;
                    Logger::logInfo("Daemon: booster (pid=%d) terminated due to signal=%d\n", pid, signal);
                    Logger::logInfo("Daemon: kill invoker process %d by signal %d \n", invoker_pid, signal);
                    if (kill(invoker_pid, signal) != 0)
                    {
                        Logger::logError("Daemon: failed to send signal to invoker: %s \n", strerror(errno));
                    }
                }
                // remove dead booster
                m_kindergarten.erase(it);
            }

            // Check if pid belongs to boosters, restart dead booster if needed
            char type = BoosterFactory::getBoosterTypeForPid(pid);
            if (type != 0)
            {
                forkBooster(type, m_boosterSleepTime);
            }
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
        Logger::logError("Daemon: Unable to fork daemon, code %d (%s)", errno, strerror(errno));
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
        Logger::logError("Daemon: Unable to fork daemon, code %d (%s)", errno, strerror(errno));
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
        Logger::logError("Daemon: Unable to create a new session, code %d (%s)", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Change the current working directory
    if ((chdir("/")) < 0)
    {
        Logger::logError("Daemon: Unable to change directory to %s, code %d (%s)", "/", errno, strerror(errno));
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
