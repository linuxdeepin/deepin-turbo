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

#include <vector>
#include <string>
#include <map>
#include <sys/types.h>

using std::vector;
using std::string;
using std::map;

class Booster;

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

    //! Don't use console for output
    void consoleQuiet();

    //! Preload libs and resolve symbols
    void preload();

    //! Daemonize flag
    bool m_daemon;

    //! Debug print flag
    bool m_quiet;

    //! Vector of current child PID's
    typedef vector<pid_t> PidVect;
    PidVect m_children;

    //! Storage of booster-invoker pid pairs
    typedef map<pid_t, pid_t> PidMap;
    PidMap m_boosterPidToInvokerPid;

    //! Pipe used to tell the parent that a new booster is needed
    int m_pipefd[2];

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

#ifdef UNIT_TEST
    friend class Ut_Daemon;
#endif
};

#endif // DAEMON_H
