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

#ifndef BOOSTER_H
#define BOOSTER_H

#include <cstdlib>
#include <string>

using std::string;

#include "appdata.h"

class Connection;
class SocketManager;

#ifdef HAVE_CREDS

    #include <sys/creds.h>
    #include <vector>
    #include <map>

    // Storage types for "binary"-formatted credentials
    typedef std::pair<creds_type_t, creds_value_t> BinCredsPair;
    typedef std::vector<BinCredsPair> CredsList;

#endif

#include <QObject>

/*!
 *  \class Booster
 *  \brief Abstract base class for all boosters (Qt-booster, M-booster and so on..)
 *
 *  Booster is a class that is used to initialize certain resources in libraries
 *  common to all applications of a type.
 *
 *  Booster also communicates with the invoker process and handles the actual
 *  jump to the main() -method of the application to be launched.
 *
 *  Booster instance dies with the launched application and a new one must be created
 *  in advance so as to launch a new application.
 */
class Booster : public QObject
{
public:

    //! Constructor
    Booster();

    //! Destructor
    virtual ~Booster();

    /*!
     * \brief Initializes the booster process.
     */
    virtual void initialize(int initialArgc, char ** initialArgv, int pipefd[2],
                            int socketFd);

    /*!
     * \brief Preload libraries.
     * Override in the custom Booster.
     */
    virtual bool preload();

    /*!
     * \brief Run the application to be invoked.
     * This method causes the application binary to be loaded
     * using dlopen(). Program execution jumps to the address of
     * "main()" found in the newly loaded library. The Booster process
     * exits with corresponding exit-code after the execution of
     * main() has finished.
     *
     * \param socketManager Pointer to the SocketManager so that
     * we can close all needless sockets in the application process.
     */
    virtual void run(SocketManager * socketManager);

    /*!
     * \brief Rename process.
     * This method overrides the argument data starting from initialArgv[0].
     * This is needed so as to get the process name and arguments displayed
     * correctly e.g. in the listing by 'ps'. initialArgv[1] may provide an
     * empty dummy space to be used. It is assumed, that the arguments are
     * located continuosly in memory and this is how it's done in glibc.
     *
     * \param initialArgc Number of the arguments of the launcher process.
     * \param initialArgv Address of the argument array of the launcher process.
     */
    void renameProcess(int initialArgc, char** initialArgv);

    /*!
     * \brief Return booster type common to all instances.
     * This is used in the simple communication between booster process.
     * and the daemon. Override in the custom Booster.
     *
     * \return A (unique) character representing the type of the Booster.
     */
    virtual char boosterType() const = 0;

    /*! Return the process name to be used when booster is not
     *  yet transformed into a running application (e.g. "booster-m"
     *  for MBooster)
     */
    virtual const string & boosterTemporaryProcessName() const = 0;

    //! Get invoker's pid
    pid_t invokersPid();

    //! Get the connection object
    Connection* connection() const;

    //! Set connection object. Booster takes the ownership.
    void setConnection(Connection * connection);

    //! Get application data object
    AppData* appData() const;

#ifdef HAVE_CREDS
    //! initialize invoker-specific credentials to be filtered out by filterOutCreds()
    static void initExtraCreds();
#endif

    /*!
     * \brief Return the communication socket used by a Booster.
     * This method returns the socket used between invoker and the Booster.
     * (common to all Boosters of the type). Override in the custom Booster.
     * \return Path to the socket file
     */
    virtual const string & socketId() const = 0;

protected:

    //! Set nice value and store the old priority. Return true on success.
    bool pushPriority(int nice);

    //! Restore the old priority stored by the previous successful setPriority().
    bool popPriority();

    /*!
     * \brief Wait for connection from invoker and read the input.
     * This method accepts a socket connection from the invoker
     * and reads the data of an application to be launched.
     *
     * \param socketFd Fd of the UNIX socket file.
     * \return true on success
     */
    virtual bool receiveDataFromInvoker(int socketFd);

    //! Sets pipe fd's used to communicate with the parent process
    void setPipeFd(int pipeFd[2]);

    //! Returns the given pipe fd (0 = read end, 1 = write end)
    int pipeFd(bool whichEnd) const;

    //! Reset out-of-memory killer adjustment
    void resetOomAdj();

private:

    //! Disable copy-constructor
    Booster(const Booster & r);

    //! Disable assignment operator
    Booster & operator= (const Booster & r);

    //! Send data to the parent process (invokers pid, respwan delay)
    //! and signal that a new booster can be created.
    void sendDataToParent();

    //! Load the library and jump to main
    int launchProcess();

    //! Helper method: load the library and find out address for "main".
    void* loadMain();

    //! Data structure representing the application to be invoked
    AppData* m_appData;

    //! Socket connection to invoker
    Connection* m_connection;

    //! Size (length) of the argument vector
    int m_argvArraySize;

    //! Process priority before pushPriority() is called
    int m_oldPriority;

    //! True if m_oldPriority is a valid value so that
    //! it can be restored later.
    bool m_oldPriorityOk;

    //! Pipe used to tell the parent that a new booster is needed
    int m_pipeFd[2];

#ifdef HAVE_CREDS
    //! filter out invoker-specific credentials from boosted application
    static void filterOutCreds(creds_t creds);

    //! set of credentials to be filtered out of credentials
    //! inhereted from invoker process
    static CredsList m_extraCreds;

    //! str array of creds to filter out
    static const char * const m_strCreds[];
#endif

#ifdef UNIT_TEST
    friend class Ut_Booster;
#endif
};

#endif // BOOSTER_H
