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
#include "boosterfactory.h"
#include "boosterpluginregistry.h"
#include "singleinstance.h"
#include "socketmanager.h"

#include <cstdlib>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <glob.h>
#include <cstring>
#include <cstdio>
#include <stdexcept>

#include "coverage.h"

Daemon * Daemon::m_instance = NULL;
int Daemon::m_lockFd = -1;
const int Daemon::m_boosterSleepTime = 2;

Daemon::Daemon(int & argc, char * argv[]) :
    m_daemon(false),
    m_quiet(false),
    m_bootMode(false),
    m_socketManager(new SocketManager),
    m_singleInstance(new SingleInstance)
{
    if (!Daemon::m_instance)
    {
        Daemon::m_instance = this;
    }
    else
    {
        throw std::runtime_error("Daemon: Daemon already created!\n");
    }

    // Parse arguments
    parseArgs(ArgVect(argv, argv + argc));

    // Disable console output
    if (m_quiet)
        consoleQuiet();

    // Store arguments list
    m_initialArgv = argv;
    m_initialArgc = argc;

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, m_boosterLauncherSocket) == -1)
    {
        throw std::runtime_error("Daemon: Creating a socket pair for boosters failed!\n");
    }

    if (pipe(m_sigPipeFd) == -1)
    {
        throw std::runtime_error("Daemon: Creating a pipe for Unix signals failed!\n");
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
        throw std::runtime_error("Daemon: Failed to open /dev/null as read-only");

    int fd = open("/dev/null", O_WRONLY);
    if ((fd == -1) || (dup(fd) < 0))
        throw std::runtime_error("Daemon: Failed to open /dev/null as write-only");
}

Daemon * Daemon::instance()
{
    return Daemon::m_instance;
}

bool Daemon::lock()
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 1;

    if((m_lockFd = open("/var/run/applauncherd.lock", O_WRONLY | O_CREAT, 0666)) == -1)
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

void Daemon::initBoosterSockets()
{
    const int numBoosters = BoosterPluginRegistry::pluginCount();
    for (int i = 0; i < numBoosters; i++)
    {
        BoosterPluginEntry * plugin = BoosterPluginRegistry::pluginEntry(i);
        if (plugin)
        {
            Logger::logDebug("Daemon: initing socket: %s", plugin->socketNameFunc());
            m_socketManager->initSocket(plugin->socketNameFunc());
        }
    }
}

void Daemon::forkBoosters()
{
    const int numBoosters = BoosterPluginRegistry::pluginCount();
    for (int i = 0; i < numBoosters; i++)
    {
        BoosterPluginEntry * plugin = BoosterPluginRegistry::pluginEntry(i);
        if (plugin)
        {
            Logger::logDebug("Daemon: forking booster: '%c'", plugin->type);
            forkBooster(plugin->type);
        }
    }
}

void Daemon::run()
{
    // Make sure that LD_BIND_NOW does not prevent dynamic linker to
    // use lazy binding in later dlopen() calls.
    unsetenv("LD_BIND_NOW");

    // dlopen() booster plugins
    loadBoosterPlugins();

    // Create sockets for each of the boosters
    initBoosterSockets();

    // dlopen single-instance
    loadSingleInstancePlugin();

    // Fork each booster for the first time
    forkBoosters();

    // Main loop
    while (true)
    {
        // Variables used by the select call
        fd_set rfds;
        int ndfs = 0;

        // Init data for select
        FD_ZERO(&rfds);

        FD_SET(m_boosterLauncherSocket[0], &rfds);
        ndfs = std::max(ndfs, m_boosterLauncherSocket[0]);

        FD_SET(m_sigPipeFd[0], &rfds);
        ndfs = std::max(ndfs, m_sigPipeFd[0]);

        // Wait for something appearing in the pipes.
        if (select(ndfs + 1, &rfds, NULL, NULL, NULL) > 0)
        {
            Logger::logDebug("Daemon: select done.");

            // Check if a booster died
            if (FD_ISSET(m_boosterLauncherSocket[0], &rfds))
            {
                Logger::logDebug("Daemon: FD_ISSET(m_boosterLauncherSocket[0])");
                readFromBoosterSocket(m_boosterLauncherSocket[0]);
            }

            // Check if we got SIGCHLD, SIGTERM, SIGUSR1 or SIGUSR2
            if (FD_ISSET(m_sigPipeFd[0], &rfds))
            {
                Logger::logDebug("Daemon: FD_ISSET(m_sigPipeFd[0])");
                char dataReceived;
                read(m_sigPipeFd[0], &dataReceived, 1);

                switch (dataReceived)
                {
                case SIGCHLD:
                    Logger::logDebug("Daemon: SIGCHLD received.");
                    reapZombies();
                    break;

                case SIGTERM:
                    Logger::logDebug("Daemon: SIGTERM received.");
                    exit(EXIT_SUCCESS);
                    break;

                case SIGUSR1:
                    Logger::logDebug("Daemon: SIGUSR1 received.");
                    enterNormalMode();
                    break;

                case SIGUSR2:
                    Logger::logDebug("Daemon: SIGUSR2 received.");
                    enterBootMode();
                    break;

                case SIGPIPE:
                    Logger::logDebug("Daemon: SIGPIPE received.");
                    break;

                default:
                    break;
                }
            }
        }
    }
}

void Daemon::readFromBoosterSocket(int fd)
{
    char booster     = 0;
    pid_t invokerPid = 0;
    int delay        = 0;
    struct msghdr   msg;
    struct cmsghdr *cmsg;
    struct iovec    iov[3];
    char buf[CMSG_SPACE(sizeof(int))];

    iov[0].iov_base = &booster;
    iov[0].iov_len  = sizeof(char);
    iov[1].iov_base = &invokerPid;
    iov[1].iov_len  = sizeof(pid_t);
    iov[2].iov_base = &delay;
    iov[2].iov_len  = sizeof(int);

    msg.msg_iov        = iov;
    msg.msg_iovlen     = 3;
    msg.msg_name       = NULL;
    msg.msg_namelen    = 0;
    msg.msg_control    = buf;
    msg.msg_controllen = sizeof(buf);

    if (recvmsg(fd, &msg, 0) >= 0)
    {
        Logger::logDebug("Daemon: booster type: %c\n", booster);
        Logger::logDebug("Daemon: invoker's pid: %d\n", invokerPid);
        Logger::logDebug("Daemon: respawn delay: %d \n", delay);
        if (invokerPid != 0)
        {
            // Store booster - invoker pid pair
            // Store booster - invoker socket pair
            pid_t boosterPid = boosterPidForType(booster);
            if (boosterPid)
            {
                cmsg = CMSG_FIRSTHDR(&msg);
                int newFd;                 
                memcpy(&newFd, CMSG_DATA(cmsg), sizeof(int));
                Logger::logDebug("Daemon: socket file descriptor: %d\n", newFd);
                m_boosterPidToInvokerPid[boosterPid] = invokerPid;
                m_boosterPidToInvokerFd[boosterPid] = newFd;
            }
        }
    }
    else
    {
        Logger::logWarning("Daemon: Nothing read from the socket\n");
    }

    // Fork a new booster of the given type

    // 2nd param guarantees some time for the just launched application
    // to start up before forking new booster. Not doing this would
    // slow down the start-up significantly on single core CPUs.

    forkBooster(booster, delay);
}

void Daemon::killProcess(pid_t pid, int signal) const
{
    if (pid > 0)
    {
        Logger::logDebug("Daemon: Killing pid %d with %d", pid, signal);
        if (kill(pid, signal) != 0)
        {
            Logger::logError("Daemon: Failed to kill %d: %s\n",
                             pid, strerror(errno));
        }
    }
}

void Daemon::loadSingleInstancePlugin()
{
    void * handle = dlopen(SINGLE_INSTANCE_PATH, RTLD_NOW);
    if (!handle)
    {
        Logger::logWarning("Daemon: dlopening single-instance failed: %s", dlerror());
    }
    else
    {
        if (m_singleInstance->validateAndRegisterPlugin(handle))
        {
            Logger::logDebug("Daemon: single-instance plugin loaded.'");
        }
        else
        {
            Logger::logWarning("Daemon: Invalid single-instance plugin: '%s'",
                               SINGLE_INSTANCE_PATH);
        }
    }
}

void Daemon::loadBoosterPlugins()
{
    const char*        PATTERN = "lib*booster.so";
    const unsigned int BUF_LEN = 256;

    if (strlen(PATTERN) + strlen(BOOSTER_PLUGIN_DIR) + 2 > BUF_LEN)
    {
        Logger::logError("Daemon: path to plugins too long");
        return;
    }

    char buffer[BUF_LEN];
    memset(buffer, 0, BUF_LEN);
    strcpy(buffer, BOOSTER_PLUGIN_DIR);
    strcat(buffer, "/");
    strcat(buffer, PATTERN);

    // get full path to all plugins
    glob_t globbuf;
    if (glob(buffer, 0, NULL, &globbuf) == 0)
    {
        for (__size_t i = 0; i < globbuf.gl_pathc; i++)
        {
            void *handle = dlopen(globbuf.gl_pathv[i], RTLD_NOW | RTLD_GLOBAL);
            if (!handle)
            {
                Logger::logWarning("Daemon: dlopening booster failed: %s", dlerror());
            }
            else
            {
                char newType = BoosterPluginRegistry::validateAndRegisterPlugin(handle);
                if (newType)
                {
                    Logger::logDebug("Daemon: Booster of type '%c' loaded.'", newType);
                }
                else
                {
                    Logger::logWarning("Daemon: Invalid booster plugin: '%s'", buffer);
                }
            }
        }
        globfree(&globbuf);
    }
    else
    {
        Logger::logError("Daemon: can't find booster plugins");
    }
}

void Daemon::forkBooster(char type, int sleepTime)
{
    // Invalidate current booster pid for the given type
    setPidToBooster(type, 0);

    // Fork a new process
    pid_t newPid = fork();

    if (newPid == -1)
        throw std::runtime_error("Daemon: Forking while invoking");

    if (newPid == 0) /* Child process */
    {
        // Reset used signal handlers
        signal(SIGCHLD, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGUSR1, SIG_DFL);
        signal(SIGUSR2, SIG_DFL);
        signal(SIGPIPE, SIG_DFL);

        // Will get this signal if applauncherd dies
        prctl(PR_SET_PDEATHSIG, SIGHUP);

        // Close unused read end of the booster socket
        close(m_boosterLauncherSocket[0]);

        // Close signal pipe
        close(m_sigPipeFd[0]);
        close(m_sigPipeFd[1]);

        // Close unused sockets inherited from daemon
        closeUnusedSockets(type);

        // Close lock file, it's not needed in the booster
        Daemon::unlock();

        // Close socket file descriptors
        FdMap::iterator i(m_boosterPidToInvokerFd.begin());
        while (i != m_boosterPidToInvokerFd.end())
        {
            if ((*i).second != -1) {
                close((*i).second);
                (*i).second = -1;
            }
            i++;
        }
        // Set session id
        if (setsid() < 0)
            Logger::logError("Daemon: Couldn't set session id\n");

        // Guarantee some time for the just launched application to
        // start up before initializing new booster if needed.
        // Not done if in the boot mode.
        if (!m_bootMode && sleepTime)
            sleep(sleepTime);

        Logger::logDebug("Daemon: Running a new Booster of type '%c'", type);

        // Create a new booster, initialize and run it
        Booster * booster = BoosterFactory::create(type);
        if (booster)
        {
            // Initialize and wait for commands from invoker
            booster->initialize(m_initialArgc, m_initialArgv, m_boosterLauncherSocket[1],
                                m_socketManager->findSocket(booster->socketId().c_str()),
                                m_singleInstance, m_bootMode);

            // Run the current Booster
            int retval = booster->run(m_socketManager);

            // Finish
            delete booster;

            // _exit() instead of exit() to avoid situation when destructors
            // for static objects may be run incorrectly
            _exit(retval);
        }
        else
        {
            throw std::runtime_error(std::string("Daemon: Unknown booster type '") + type + "'");
        }
    }
    else /* Parent process */
    {
        // Store the pid so that we can reap it later
        m_children.push_back(newPid);

        // Set current process ID globally to the given booster type
        // so that we now which booster to restart when booster exits.
        setPidToBooster(type, newPid);
    }
}

void Daemon::reapZombies()
{
    // Loop through all child pid's and wait for them with WNOHANG.
    PidVect::iterator i(m_children.begin());
    while (i != m_children.end())
    {
        // Check if the pid had exited and become a zombie
        int status;
        pid_t pid = waitpid(*i, &status, WNOHANG);
        if (pid)
        {
            // The pid had exited. Remove it from the pid vector.
            i = m_children.erase(i);

            // Find out if the exited process has a mapping with an invoker process.
            // If this is the case, then kill the invoker process with the same signal
            // that killed the exited process.
            PidMap::iterator it = m_boosterPidToInvokerPid.find(pid);
            if (it != m_boosterPidToInvokerPid.end())
            {
                Logger::logDebug("Daemon: Terminated process had a mapping to an invoker pid");

                if (WIFEXITED(status))
                {
                    Logger::logDebug("Daemon: child exited by exit(x), _exit(x) or return x\n");
                    Logger::logDebug("Daemon: x == %d\n", WEXITSTATUS(status));
                    FdMap::iterator fd = m_boosterPidToInvokerFd.find(pid);
                    if (fd != m_boosterPidToInvokerFd.end())
                    {
                        write((*fd).second, &INVOKER_MSG_EXIT, sizeof(uint32_t));
                        int exitStatus = WEXITSTATUS(status);
                        write((*fd).second, &exitStatus, sizeof(int));
                        close((*fd).second);
                        m_boosterPidToInvokerFd.erase(fd);
                    }
                }
                else if (WIFSIGNALED(status))
                {
                    int signal = WTERMSIG(status);
                    pid_t invokerPid = (*it).second;

                    Logger::logDebug("Daemon: Booster (pid=%d) was terminated due to signal %d\n", pid, signal);
                    Logger::logDebug("Daemon: Killing invoker process (pid=%d) by signal %d..\n", invokerPid, signal);

                    FdMap::iterator fd = m_boosterPidToInvokerFd.find(pid);
                    if (fd != m_boosterPidToInvokerFd.end())
                    {
                        close((*fd).second);
                        m_boosterPidToInvokerFd.erase(fd);
                    }

                    killProcess(invokerPid, signal);
                }

                // Remove a dead booster
                m_boosterPidToInvokerPid.erase(it);
            }

            // Check if pid belongs to a booster and restart the dead booster if needed
            char type = boosterTypeForPid(pid);
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

void Daemon::setPidToBooster(char type, pid_t pid)
{
    m_boosterTypeToPid[type] = pid;
}

char Daemon::boosterTypeForPid(pid_t pid) const
{
    TypeMap::const_iterator i = m_boosterTypeToPid.begin();
    while (i != m_boosterTypeToPid.end())
    {
        if (i->second == pid)
        {
            return i->first;
        }

        i++;
    }

    return 0;
}

pid_t Daemon::boosterPidForType(char type) const
{
    TypeMap::const_iterator i = m_boosterTypeToPid.find(type);
    return i == m_boosterTypeToPid.end() ? 0 : i->second;
}

void Daemon::closeUnusedSockets(char type)
{
    const int numBoosters = BoosterPluginRegistry::pluginCount();
    for (int i = 0; i < numBoosters; i++)
    {
        BoosterPluginEntry * plugin = BoosterPluginRegistry::pluginEntry(i);
        if (plugin && (plugin->type != type))
        {
            m_socketManager->closeSocket(plugin->socketNameFunc());
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
        throw std::runtime_error("Daemon: Unable to fork daemon");

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    // Fork off the parent process: second fork
    pid = fork();
    if (pid < 0)
        throw std::runtime_error("Daemon: Unable to fork daemon");

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    // Check the lock
    if(!Daemon::lock())
        throw std::runtime_error(std::string(PROG_NAME_LAUNCHER) + " is already running\n");

    // Change the file mode mask
    umask(0);

    // Open any logs here

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0)
        throw std::runtime_error("Daemon: Unable to setsid.");

    // Change the current working directory
    if ((chdir("/")) < 0)
        throw std::runtime_error("Daemon: Unable to chdir to '/'");

    // Open file descriptors pointing to /dev/null
    // Redirect standard file descriptors to /dev/null
    // Close new file descriptors

    const int new_stdin = open("/dev/null", O_RDONLY);
    if (new_stdin != -1) {
        dup2(new_stdin, STDIN_FILENO);
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
        if ((*i) == "--boot-mode" || (*i) == "-b")
        {
            Logger::logInfo("Daemon: Boot mode set.");
            m_bootMode = true;
        }
        else if ((*i) == "--daemon" || (*i) == "-d")
        {
            m_daemon = true;
        }
        else if ((*i) == "--debug")
        {
            Logger::setDebugMode(true);
        }
        else if ((*i) == "--help" || (*i) == "-h")
        {
            usage(EXIT_SUCCESS);
        }
        else if ((*i) == "--quiet" || (*i) == "-q")
        {
            m_quiet = true;
        }
    }
}

// Prints the usage and exits with given status
void Daemon::usage(int status)
{
    printf("\nUsage: %s [options]\n\n"
           "Start the application launcher daemon.\n\n"
           "Options:\n"
           "  -b, --boot-mode  Start %s in the boot mode. This means that\n"
           "                   boosters will not initialize caches and booster\n"
           "                   respawn delay is set to zero.\n"
           "                   Normal mode is restored by sending SIGUSR1\n"
           "                   to the launcher.\n"
           "                   Boot mode can be activated also by sending SIGUSR2\n"
           "                   to the launcher.\n"
           "  -d, --daemon     Run as %s a daemon.\n"
           "  --debug          Enable debug messages and log everything also to stdout.\n"
           "  -q, --quiet      Close fd's 0, 1 and 2.\n"
           "  -h, --help       Print this help.\n\n",
           PROG_NAME_LAUNCHER, PROG_NAME_LAUNCHER, PROG_NAME_LAUNCHER);

    exit(status);
}

int Daemon::sigPipeFd() const
{
    return m_sigPipeFd[1];
}

void Daemon::enterNormalMode()
{
    if (m_bootMode)
    {
        m_bootMode = false;

        // Kill current boosters
        killBoosters();

        Logger::logInfo("Daemon: Exited boot mode.");
    }
    else
    {
        Logger::logInfo("Daemon: Already in normal mode.");
    }
}

void Daemon::enterBootMode()
{
    if (!m_bootMode)
    {
        m_bootMode = true;

        // Kill current boosters
        killBoosters();

        Logger::logInfo("Daemon: Entered boot mode.");
    }
    else
    {
        Logger::logInfo("Daemon: Already in boot mode.");
    }
}

void Daemon::killBoosters()
{
    TypeMap::iterator iter(m_boosterTypeToPid.begin());
    while (iter != m_boosterTypeToPid.end())
    {
        killProcess(iter->second, SIGTERM);
        iter++;
    }

    // NOTE!!: m_boosterTypeToPid must not be cleared
    // in order to automatically start new boosters.
}

Daemon::~Daemon()
{
    delete m_socketManager;
    delete m_singleInstance;

#ifdef WITH_COVERAGE
    __gcov_flush();
#endif
}
