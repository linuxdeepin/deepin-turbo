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

#include "booster.h"
#include "connection.h"
#include "logger.h"

#include <cstdlib>
#include <dlfcn.h>
#include <cerrno>
#include <unistd.h>
#include <sys/user.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#ifdef HAVE_CREDS
    #include <sys/creds.h>
#endif

Booster::Booster() :
    m_conn(NULL),
    m_argvArraySize(0),
    m_oldPriority(0),
    m_oldPriorityOk(false)
{}

Booster::~Booster()
{
    if (m_conn != NULL)
    {
        delete m_conn;
        m_conn = NULL;
    }
}

bool Booster::preload()
{
    return true;
}

void Booster::initialize(int initialArgc, char ** initialArgv, int newPipeFd[2])
{
    setPipeFd(newPipeFd);

    // Drop priority (nice = 10)
    pushPriority(10);

    // Preload stuff
    preload();

    // Clean-up all the env variables
    clearenv();

    // Rename process to temporary booster process name, e.g. "booster-m"
    renameProcess(initialArgc, initialArgv);

    // Restore priority
    popPriority();

    // Wait and read commands from the invoker
    Logger::logNotice("Daemon: Wait for message from invoker");
    readCommand();

    // Give the process the real application name now that it
    // has been read from invoker in readCommand().
    renameProcess(initialArgc, initialArgv);

    // Signal the parent process that it can create a new
    // waiting booster process and close write end
    const char msg = boosterType();
    ssize_t ret = write(pipeFd(1), reinterpret_cast<const void *>(&msg), 1);
    if (ret == -1) {
        Logger::logError("Booster: Couldn't send type message to launcher process\n");
    }

    // Send to the parent process pid of invoker for tracking
    pid_t pid = invokersPid();
    ret = write(pipeFd(1), reinterpret_cast<const void *>(&pid), sizeof(pid_t));
    if (ret == -1) {
        Logger::logError("Booster: Couldn't send invoker's pid to launcher process\n");
    }

    // Send to the parent process booster respawn delay value
    int delay = m_app.delay();
    ret = write(pipeFd(1), reinterpret_cast<const void *>(&delay), sizeof(int));
    if (ret == -1) {
        Logger::logError("Booster: Couldn't send respawn delay value to launcher process\n");
    }

    // close pipe
    close(pipeFd(1));

    // Don't care about fate of parent applauncherd process any more
    prctl(PR_SET_PDEATHSIG, 0);

    // Set dumpable flag
    prctl(PR_SET_DUMPABLE, 1);
}

bool Booster::readCommand()
{
    // Setup the conversation channel with the invoker.
    m_conn = new Connection(socketId());

    // Accept a new invocation.
    if (m_conn->acceptConn(m_app))
    {
        bool res = m_conn->receiveApplicationData(m_app);
        if(!res)
        {
            m_conn->closeConn();
            return false;
        }

        if (!m_conn->isReportAppExitStatusNeeded())
        {
            m_conn->closeConn();
        }
        return true;
    }
    return false;
}

void Booster::run()
{
    if (!m_app.fileName().empty())
    {
        //check if can close sockets here
        if (!m_conn->isReportAppExitStatusNeeded())
        {
            Connection::closeAllSockets();
        }

        Logger::logInfo("Booster: invoking '%s' ", m_app.fileName().c_str());
        int ret_val = launchProcess();

        if (m_conn->isReportAppExitStatusNeeded())
        {
            m_conn->sendAppExitStatus(ret_val);
            m_conn->closeConn();
            Connection::closeAllSockets();
        }

    }
    else
    {
        Logger::logError("Booster: nothing to invoke\n");
    }
}

void Booster::renameProcess(int parentArgc, char** parentArgv)
{
    if (m_argvArraySize == 0)
    {
        // rename process for the first time
        // calculate and store size of parentArgv array

        for (int i = 0; i < parentArgc; i++)
            m_argvArraySize += strlen(parentArgv[i]) + 1;

        m_argvArraySize--;
    }

    if (m_app.appName().empty())
    {
        // application name isn't known yet, let's give to the process
        // temporary booster name
        m_app.setAppName(boosterTemporaryProcessName());
    }

    const char* newProcessName = m_app.appName().c_str();
    Logger::logNotice("Booster: set new name for process: %s", newProcessName);
    
    // This code copies all the new arguments to the space reserved
    // in the old argv array. If an argument won't fit then the algorithm
    // leaves it fully out and terminates.
    
    int spaceAvailable = m_argvArraySize;
    if (spaceAvailable > 0)
    {
        memset(parentArgv[0], '\0', spaceAvailable);
        strncat(parentArgv[0], newProcessName, spaceAvailable);
        
        spaceAvailable -= strlen(parentArgv[0]);
        
        for (int i = 1; i < m_app.argc(); i++)
        {
            if (spaceAvailable > static_cast<int>(strlen(m_app.argv()[i])) + 1)
            {
                strncat(parentArgv[0], " ", 1);
                strncat(parentArgv[0], m_app.argv()[i], spaceAvailable);
                spaceAvailable -= strlen(m_app.argv()[i] + 1);
            }
            else
            {
                break;
            }
        }
    }

    // Set the process name using prctl, killall and top use it
    if ( prctl(PR_SET_NAME, basename(newProcessName)) == -1 )
        Logger::logError("Booster: on set new process name: %s ", strerror(errno));

    setenv("_", newProcessName, true);
}

int Booster::launchProcess()
{
    // Possibly restore process priority
    errno = 0;
    const int cur_prio = getpriority(PRIO_PROCESS, 0);
    if (!errno && cur_prio < m_app.priority())
        setpriority(PRIO_PROCESS, 0, m_app.priority());

    // Set user ID and group ID of calling process if differing
    // from the ones we got from invoker

    if (getuid() != m_app.userId())
        setuid(m_app.userId());

    if (getgid() != m_app.groupId())
        setgid(m_app.groupId());

    // Load the application and find out the address of main()
    void* handle = loadMain();

    // Duplicate I/O descriptors
    for (unsigned int i = 0; i < m_app.ioDescriptors().size(); i++)
        if (m_app.ioDescriptors()[i] > 0)
            dup2(m_app.ioDescriptors()[i], i);

    // Set PWD
    const char * pwd = getenv("PWD");
    if (pwd)
    {
        chdir(pwd);
    }

    Logger::logNotice("Booster: launching process: '%s' ", m_app.fileName().c_str());

    // Close logger
    Logger::closeLog();

    // Jump to main()
    const int retVal = m_app.entry()(m_app.argc(), const_cast<char **>(m_app.argv()));
    m_app.deleteArgv();
    dlclose(handle);
    return retVal;
}

void* Booster::loadMain()
{
#ifdef HAVE_CREDS
    // Set application's platform security credentials
    int err = creds_confine2(m_app.fileName().c_str(), credp_str2flags("set", NULL), m_app.peerCreds());
    m_app.deletePeerCreds();

    if (err < 0)
    {
        // Credential setup has failed, abort.
        Logger::logErrorAndDie(EXIT_FAILURE, "Booster: Failed to setup credentials for launching application: %d\n", err);
    }
#endif

    // Load the application as a library
    void * module = dlopen(m_app.fileName().c_str(), RTLD_LAZY | RTLD_GLOBAL);

    if (!module)
        Logger::logErrorAndDie(EXIT_FAILURE, "Booster: Loading invoked application failed: '%s'\n", dlerror());

    // Find out the address for symbol "main". dlerror() is first used to clear any old error conditions,
    // then dlsym() is called, and then dlerror() is checked again. This procedure is documented
    // in dlsym()'s man page.

    dlerror();
    m_app.setEntry(reinterpret_cast<entry_t>(dlsym(module, "main")));

    const char * error_s = dlerror();
    if (error_s != NULL)
        Logger::logErrorAndDie(EXIT_FAILURE, "Booster: Loading symbol 'main' failed: '%s'\n", error_s);

    return module;
}

bool Booster::pushPriority(int nice)
{
    errno = 0;
    m_oldPriorityOk = true;
    m_oldPriority   = getpriority(PRIO_PROCESS, getpid());

    if (errno)
    {
        m_oldPriorityOk = false;
    }
    else
    {
        return setpriority(PRIO_PROCESS, getpid(), nice) != -1;
    }

    return false;
}

bool Booster::popPriority()
{
    if (m_oldPriorityOk)
    {
        return setpriority(PRIO_PROCESS, getpid(), m_oldPriority) != -1;
    }

    return false;
}

pid_t Booster::invokersPid()
{
    if (m_conn->isReportAppExitStatusNeeded())
    {
        return m_conn->peerPid();
    }
    else
    {
        return 0;
    }
}

void Booster::setPipeFd(int newPipeFd[2])
{
    m_pipeFd[0] = newPipeFd[0];
    m_pipeFd[1] = newPipeFd[1];
}

int Booster::pipeFd(bool whichEnd) const
{
    return m_pipeFd[whichEnd];
}

