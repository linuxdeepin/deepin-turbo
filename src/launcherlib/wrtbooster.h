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

#ifndef WRTBOOSTER_H
#define WRTBOOSTER_H

#include "booster.h"

/*!
    \class WRTBooster
    \brief Booster for web runtime applications running on top of MeeGo Touch.

    WRTBooster effectively fills MComponentCache with fresh objects
    similarly to MBooster. However, the cache content is optimized for
    web runtime's use.
 */
class WRTBooster : public Booster
{
public:

    //! \brief Constructor
    WRTBooster() {};

    //! \brief Destructor
    virtual ~WRTBooster() {};

    //! \reimp
    virtual bool preload();

    /*!
     * \brief Return the socket name common to all WRTBooster objects.
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
     * \brief Return a unique character ('d') represtenting the type of WRTBoosters.
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

protected:

    //! \reimp
    virtual const string & socketId() const;

private:

    //! Disable copy-constructor
    WRTBooster(const WRTBooster & r);

    //! Disable assignment operator
    WRTBooster & operator= (const WRTBooster & r);

    static const string m_socketId;

    static int m_ProcessID;

    //! Process name to be used when booster is not
    //! yet transformed into a running application
    static const string m_temporaryProcessName;

#ifdef UNIT_TEST
    friend class Ut_WRTBooster;
#endif
};

#endif // WRTBOOSTER_H
