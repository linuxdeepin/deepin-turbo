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

#ifndef MBOOSTER_H
#define MBOOSTER_H

#include "booster.h"
#include <QObject>
#include <QSocketNotifier>
#include <MGConfItem>
#include <tr1/memory>

using std::tr1::shared_ptr;

#include <signal.h>

/*!
    \class MBooster
    \brief MeeGo Touch -specific version of the Booster.

    MBooster effectively fills MComponentCache with fresh objects.
    MeeGo Touch applications can then try to use already initialized objects 
    from MComponentCache. This can significantly reduce the startup time of a 
    MeeGo Touch application.
 */
class MBooster : public QObject, public Booster
{
    Q_OBJECT

public:

    //! \brief Constructor
    MBooster();

    //! \brief Destructor
    virtual ~MBooster() {};

    //! \reimp
    virtual bool preload();

    /*!
     * \brief Return the socket name common to all MBooster objects.
     * \return Path to the socket file.
     */
    static const string & socketName();

    //! Return the process name to be used when booster is not
    //! yet transformed into a running application
    static const string & temporaryProcessName();

    //! \reimp
    virtual const string & boosterTemporaryProcessName() const;

    //! \reimp
    virtual char boosterType() const { return type(); }

    /*!
     * \brief Return a unique character ('d') represtenting the type of MBoosters.
     * \return Type character.
     */
    static char type();

    /*!
     * \brief Keep booster pid, should be reset before booster run application's main() function
     */
    static void setProcessId(int pid);

    /*!
     * \brief Return booster pid
     */
    static int processId();

    //! UNIX signal handler for SIGHUP
    static void hupSignalHandler(int unused);

    //! Setup UNIX signal handlers
    static bool setupUnixSignalHandlers();

    //! Restore UNIX signal handlers to previous values
    static bool restoreUnixSignalHandlers();

protected:

    //! \reimp
    virtual const string & socketId() const;

    //! \reimp
    virtual bool readCommand();

private:

    //! Disable copy-constructor
    MBooster(const MBooster & r);

    //! Disable assignment operator
    MBooster & operator= (const MBooster & r);

    static const string m_socketId;

    static int m_ProcessID;

    //! Process name to be used when booster is not
    //! yet transformed into a running application
    static const string m_temporaryProcessName;

    //! wait for socket connection
    void accept();

    //! Socket pair used to get SIGHUP
    static int m_sighupFd[2];

    //! Socket notifier used for m_sighupFd
    shared_ptr<QSocketNotifier> m_snHup;

    //! Old sigaction struct
    static struct sigaction m_oldSigAction;

    //! GConf item to listen theme change
    MGConfItem* m_item;

private slots:

    //! Qt signal handler for SIGHUP.
    void handleSigHup();

    //! Qt signal handler for theme change
    void notifyThemeChange();


signals:

    void connectionAccepted();

#ifdef UNIT_TEST
    friend class Ut_MBooster;
#endif
};

#endif // MBOOSTER_H
