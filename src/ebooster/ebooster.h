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

#ifndef EBOOSTER_H
#define EBOOSTER_H

#include "booster.h"
#include <tr1/memory>

using std::tr1::shared_ptr;

/*!
    \class EBooster
    \brief EBooster is a "booster" that only exec()'s the given binary.

    This can be used with e.g. splash screen to launch any application.
 */
class EBooster : public Booster
{
public:

    //! \brief Constructor
    EBooster() {}

    //! \brief Destructor
    virtual ~EBooster() {}

    /*!
     * \brief Return the socket name common to all EBooster objects.
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
     * \brief Return a unique character ('e') represtenting the type of EBoosters.
     * \return Type character.
     */
    static char type();

    //! \reimp
    virtual const string & socketId() const;

protected:

    //! \reimp
    virtual int launchProcess();

    //! \reimp
    virtual bool preload();

private:

    //! Disable copy-constructor
    EBooster(const EBooster & r);

    //! Disable assignment operator
    EBooster & operator= (const EBooster & r);

    static const string m_socketId;

    //! Process name to be used when booster is not
    //! yet transformed into a running application
    static const string m_temporaryProcessName;

    //! wait for socket connection
    void accept();

#ifdef UNIT_TEST
    friend class Ut_EBooster;
#endif
};

#endif // EBooster_H
