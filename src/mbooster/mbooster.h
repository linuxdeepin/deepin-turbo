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

#include "eventhandler.h"
#include "booster.h"

#include <signal.h>

/*!
    \class MBooster
    \brief MeeGo Touch -specific version of the Booster.

    MBooster effectively fills MComponentCache with fresh objects.
    MeeGo Touch applications can then try to use already initialized objects 
    from MComponentCache. This can significantly reduce the startup time of a 
    MeeGo Touch application.
 */
class MBooster : public Booster
{
public:

    //! \brief Constructor
    MBooster() {}

    //! \brief Destructor
    virtual ~MBooster() {}

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
     * \brief Return a unique character ('m') represtenting the type of MBoosters.
     * \return Type character.
     */
    static char type();

    //! \reimp
    virtual const string & socketId() const;

protected:

    //! \reimp
    virtual bool preload();

    //! \reimp
    virtual bool receiveDataFromInvoker(int socketFd);

private:

    //! Disable copy-constructor
    MBooster(const MBooster & r);

    //! Disable assignment operator
    MBooster & operator= (const MBooster & r);

    static const string m_socketId;

    //! Process name to be used when booster is not
    //! yet transformed into a running application
    static const string m_temporaryProcessName;

    //! wait for socket connection
    void accept();

private slots:

    //! Qt signal handler for SIGHUP.
    void handleSigHup();

    //! Qt signal handler for theme change
    void notifyThemeChange();


#ifdef UNIT_TEST
    friend class Ut_MBooster;
#endif
};

#endif // MBOOSTER_H
