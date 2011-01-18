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

#ifndef DAEMON_H
#define DAEMON_H

#include <string>

using std::string;

#include <sys/types.h>
#include <tr1/memory>

using std::tr1::shared_ptr;

#include <vector>

using std::vector;

#include <map>

using std::map;

#include <signal.h>

class Booster;
class SocketManager;
class SingleInstance;

/*!
 * \class Daemon.
 * \brief Daemon wraps up the daemonizing functionality.
 *
 * Daemon wraps up the daemonizing functionality and is the
 * main object of the launcher program. It runs the main loop of the
 * application, listens connections from the invoker and forks Booster
 * processes.
 */
class Daemon
{
public:

    /*!
     * \brief Constructor
     * \param argc Argument count delivered to main()
     * \param argv Argument array delivered to main()
     *
     * Supported arguments:
     * --daemon == daemonize
     * --quiet  == quiet
     * --help   == print usage
     */
    Daemon(int & argc, char * argv[]);

    //! Destructor
    ~Daemon();

    /*!
     * \brief Run main loop and fork Boosters.
     */
    void run();

    /*! \brief Return the one-and-only Daemon instance.
     * \return Pointer to the Daemon instance.
     */
    static Daemon * instance();

    //! \brief Reapes children processes gone zombies (finished Boosters).
    void reapZombies();

    //! Lock file to prevent launch of second instance
    static bool lock();

    //! Unlock file (lock is not needed in boosters)
    static void unlock();

    //! Get fd to write when SIGCHLD arrives
    int sigChldPipeFd() const;

    //! Get fd to write when SIGTERM arrives
    int sigTermPipeFd() const;

    //! Get fd to write when SIGUSR1 arrives
    int sigUsr1PipeFd() const;

private:

    //! Disable copy-constructor
    Daemon(const Daemon & r);

    //! Disable assignment operator
    Daemon & operator= (const Daemon & r);

    //! Parse arguments
    typedef vector<string> ArgVect;
    void parseArgs(const ArgVect & args);

    //! Fork to a daemon
    void daemonize();

    //! Fork process that kills boosters if needed
    void forkKiller();

    //! Forks and initializes a new Booster
    void forkBooster(char type, int sleepTime = 0);

    //! Init sockects used to communicate with boosters
    void initBoosterSockets();

    //! Fork all registered boosters
    void forkBoosters();

    //! Don't use console for output
    void consoleQuiet();

    //! Kill given pid with SIGKILL by default
    void killProcess(pid_t pid, int signal = SIGKILL) const;

    //! Load (dlopen()) booster plugins
    void loadBoosterPlugins();

    //! Load single-instance plugin
    void loadSingleInstancePlugin();

    //! Assign given pid to given booster
    void setPidToBooster(char type, pid_t pid);

    //! Return booster type for given pid. Return 0 if fails.
    char boosterTypeForPid(pid_t pid) const;

    //! Return pid for given booster type. Return 0 if fails.
    pid_t boosterPidForType(char type) const;

    //! Close all sockets NOT used by the given booster type.
    void closeUnusedSockets(char type);

    //! Read and process data from a booster pipe
    void readFromBoosterPipe(int fd);

    //! Enter normal mode (restart boosters with cache enabled)
    void enterNormalMode();

    //! Kill all active boosters with -9
    void killBoosters();

    //! Prints the usage and exits with given status
    void usage(int status);

    //! Daemonize flag (--fork). Daemon forks if true.
    bool m_daemon;

    //! Debug print flag (--quiet). Daemon closes fd's 0 - 2 if true.
    bool m_quiet;

    /*! Flag indicating boot mode (--boot-mode). If true, then:
     *  - Caches won't be initialized.
     *  - Booster respwan delay is 0.
     *
     *  Normal mode is activated by firing SIGUSR1.
     */
    bool m_bootMode;

    //! Vector of current child PID's
    typedef vector<pid_t> PidVect;
    PidVect m_children;

    //! Storage of booster <-> invoker pid pairs
    typedef map<pid_t, pid_t> PidMap;
    PidMap m_boosterPidToInvokerPid;

    //! Mapping for booster type <-> pid
    typedef map<char, pid_t> TypeMap;
    TypeMap m_boosterTypeToPid;

    //! Pipe used to tell the parent that a new booster is needed +
    //! some parameters.
    int m_boosterPipeFd[2];

    //! Pipe used to safely catch SIGCHLD
    int m_sigChldPipeFd[2];

    //! Pipe used to safely catch SIGTERM
    int m_sigTermPipeFd[2];

    //! Pipe used to safely catch SIGUSR1
    int m_sigUsr1PipeFd[2];

    //! Argument vector initially given to the launcher process
    int m_initialArgc;

    //! Argument count initially given to the launcher process
    char** m_initialArgv;

    //! Singleton Daemon instance
    static Daemon * m_instance;

    //! File descriptor of the lock file
    static int m_lockFd;

    //! Time to sleep before forking a new booster
    static const int m_boosterSleepTime;

    //! Manager for invoker <-> booster sockets
    SocketManager * m_socketManager;

    //! Single instance plugin handle
    SingleInstance * m_singleInstance;

#ifdef UNIT_TEST
    friend class Ut_Daemon;
#endif
};

#endif // DAEMON_H
