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
#include <fcntl.h>

#ifdef HAVE_CREDS
    #include <sys/creds.h>

    CredsList Booster::m_extraCreds;

    const char * const Booster::m_strCreds[] = {
        "applauncherd-launcher::access",
        "SRC::com.nokia.maemo",
        "AID::com.nokia.maemo.applauncherd-invoker.client",
        "applauncherd-invoker::applauncherd-invoker"
    };
#endif

Booster::Booster() :
    m_appData(new AppData),
    m_connection(NULL),
    m_argvArraySize(0),
    m_oldPriority(0),
    m_oldPriorityOk(false)
{}

Booster::~Booster()
{
    delete m_connection;
    m_connection = NULL;

    delete m_appData;
    m_appData = NULL;
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

    // Rename process to temporary booster process name, e.g. "booster-m"
    renameProcess(initialArgc, initialArgv);

    // Restore priority
    popPriority();

    // Wait and read commands from the invoker
    Logger::logNotice("Booster: Wait for message from invoker");
    if (!receiveDataFromInvoker()) {
        Logger::logError("Booster: Couldn't read command\n");
    }

    // Give the process the real application name now that it
    // has been read from invoker in receiveDataFromInvoker().
    renameProcess(initialArgc, initialArgv);

    // Send parent process a message that it can create a new booster,
    // send pid of invoker, send booster respawn value
    sendDataToParent();

    // close pipe
    close(pipeFd(1));

    // Don't care about fate of parent applauncherd process any more
    prctl(PR_SET_PDEATHSIG, 0);

    // Set dumpable flag
    prctl(PR_SET_DUMPABLE, 1);
}

void Booster::sendDataToParent()
{
    // Signal the parent process that it can create a new
    // waiting booster process and close write end
    const char msg = boosterType();
    if (write(pipeFd(1), reinterpret_cast<const void *>(&msg), 1) == -1) {
        Logger::logError("Booster: Couldn't send type message to launcher process\n");
    }

    // Send to the parent process pid of invoker for tracking
    pid_t pid = invokersPid();
    if (write(pipeFd(1), reinterpret_cast<const void *>(&pid), sizeof(pid_t)) == -1) {
        Logger::logError("Booster: Couldn't send invoker's pid to launcher process\n");
    }

    // Send to the parent process booster respawn delay value
    int delay = m_appData->delay();
    if (write(pipeFd(1), reinterpret_cast<const void *>(&delay), sizeof(int)) == -1) {
        Logger::logError("Booster: Couldn't send respawn delay value to launcher process\n");
    }
}

bool Booster::receiveDataFromInvoker()
{
    // Setup the conversation channel with the invoker.
    m_connection = new Connection(socketId());

    // Accept a new invocation.
    if (m_connection->accept(m_appData))
    {
        // Receive application data from the invoker
        if(!m_connection->receiveApplicationData(m_appData))
        {
            m_connection->close();
            return false;
        }

        // Close the connection if exit status doesn't need
        // to be sent back to invoker
        if (!m_connection->isReportAppExitStatusNeeded())
        {
            m_connection->close();
        }
        return true;
    }

    return false;
}

void Booster::run()
{
    if (!m_appData->fileName().empty())
    {
        // Check if can close sockets already here
        if (!m_connection->isReportAppExitStatusNeeded())
        {
            Connection::closeAllSockets();
        }

        // Execute the binary
        Logger::logInfo("Booster: invoking '%s' ", m_appData->fileName().c_str());
        int ret_val = launchProcess();

        // Send exit status to invoker, if needed
        if (m_connection->isReportAppExitStatusNeeded())
        {
            m_connection->sendAppExitStatus(ret_val);
            m_connection->close();
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

    if (m_appData->appName().empty())
    {
        // application name isn't known yet, let's give to the process
        // temporary booster name
        m_appData->setAppName(boosterTemporaryProcessName());
    }

    const char* newProcessName = m_appData->appName().c_str();
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
        
        for (int i = 1; i < m_appData->argc(); i++)
        {
            if (spaceAvailable > static_cast<int>(strlen(m_appData->argv()[i])) + 1)
            {
                strncat(parentArgv[0], " ", 1);
                strncat(parentArgv[0], m_appData->argv()[i], spaceAvailable);
                spaceAvailable -= strlen(m_appData->argv()[i] + 1);
            }
            else
            {
                break;
            }
        }
    }

    // Set the process name using prctl, 'killall' and 'top' use it
    if ( prctl(PR_SET_NAME, basename(newProcessName)) == -1 )
        Logger::logError("Booster: on set new process name: %s ", strerror(errno));

    setenv("_", newProcessName, true);
}

int Booster::launchProcess()
{
    // Possibly restore process priority
    errno = 0;
    const int cur_prio = getpriority(PRIO_PROCESS, 0);
    if (!errno && cur_prio < m_appData->priority())
        setpriority(PRIO_PROCESS, 0, m_appData->priority());

    // Set user ID and group ID of calling process if differing
    // from the ones we got from invoker

    if (getuid() != m_appData->userId())
        setuid(m_appData->userId());

    if (getgid() != m_appData->groupId())
        setgid(m_appData->groupId());

    // Reset out-of-memory killer adjustment
    resetOomAdj();

#ifdef HAVE_CREDS
    // filter out invoker-specific credentials
    Booster::filterOutCreds(m_appData->peerCreds());

    // Set application's platform security credentials.
    // creds_confine2() tries first to use application-specific credentials, but if they are missing
    // from the system, it uses credentials inherited from the invoker.
    int err = creds_confine2(m_appData->fileName().c_str(), credp_str2flags("set", NULL), m_appData->peerCreds());
    m_appData->deletePeerCreds();

    if (err < 0)
    {
        // Credential setup has failed, abort.
        Logger::logErrorAndDie(EXIT_FAILURE, "Booster: Failed to setup credentials for launching application: %d\n", err);
    }
#endif

    // Load the application and find out the address of main()
    void* handle = loadMain();

    // Duplicate I/O descriptors
    for (unsigned int i = 0; i < m_appData->ioDescriptors().size(); i++)
    {
        if (m_appData->ioDescriptors()[i] > 0)
        {
            dup2(m_appData->ioDescriptors()[i], i);
            close(m_appData->ioDescriptors()[i]);
        }
    }

    // Set PWD
    const char * pwd = getenv("PWD");
    if (pwd) chdir(pwd);

    Logger::logNotice("Booster: launching process: '%s' ", m_appData->fileName().c_str());
    Logger::closeLog();

    // Jump to main()
    const int retVal = m_appData->entry()(m_appData->argc(), const_cast<char **>(m_appData->argv()));
    m_appData->deleteArgv();
    dlclose(handle);
    return retVal;
}

void* Booster::loadMain()
{
    // Setup flags for dlopen

    int dlopenFlags = RTLD_LAZY;

    if (m_appData->dlopenGlobal())
        dlopenFlags |= RTLD_GLOBAL;
    else
        dlopenFlags |= RTLD_LOCAL;

#if (PLATFORM_ID == Linux)
    if (m_appData->dlopenDeep())
        dlopenFlags |= RTLD_DEEPBIND;
#endif

    // Load the application as a library
    void * module = dlopen(m_appData->fileName().c_str(), dlopenFlags);

    if (!module)
        Logger::logErrorAndDie(EXIT_FAILURE, "Booster: Loading invoked application failed: '%s'\n", dlerror());

    // Find out the address for symbol "main". dlerror() is first used to clear any old error conditions,
    // then dlsym() is called, and then dlerror() is checked again. This procedure is documented
    // in dlsym()'s man page.

    dlerror();
    m_appData->setEntry(reinterpret_cast<entry_t>(dlsym(module, "main")));

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
    if (m_connection->isReportAppExitStatusNeeded())
    {
        return m_connection->peerPid();
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

Connection* Booster::connection() const
{
    return m_connection;
}

void Booster::setConnection(Connection * newConnection)
{
    delete m_connection;
    m_connection = newConnection;
}

AppData* Booster::appData() const
{
    return m_appData;
}

#ifdef HAVE_CREDS

void Booster::initExtraCreds()
{
    // Convert string-formatted credentials into
    // "binary"-formatted credentials

    unsigned int numCreds = sizeof(m_strCreds) / sizeof(char*);
    for (unsigned int i = 0; i < numCreds; i++)
    {
        creds_value_t value;
        creds_value_t ret = creds_str2creds(m_strCreds[i], &value);

        if (ret != CREDS_BAD)
            m_extraCreds.push_back(BinCredsPair(ret, value));
    }
}

void Booster::filterOutCreds(creds_t creds)
{
    for(unsigned int i = 0; i < m_extraCreds.size(); i++)
    {
        creds_sub(creds, m_extraCreds.at(i).first, m_extraCreds.at(i).second);
    }
}

#endif //HAVE_CREDS

void Booster::resetOomAdj()
{
    const char * PROC_OOM_ADJ_FILE = "/proc/self/oom_adj";
    int fd = open(PROC_OOM_ADJ_FILE, O_WRONLY);
    if (fd != -1)
    {
        if (write(fd, "0", sizeof(char)) == -1)
        {
            Logger::logError("Couldn't write to '%s': %s", PROC_OOM_ADJ_FILE,
                             strerror(errno));
        }

        close(fd);
    }
    else
    {
        Logger::logError("Couldn't open '%s' for write: %s", PROC_OOM_ADJ_FILE,
                         strerror(errno));
    }
}
