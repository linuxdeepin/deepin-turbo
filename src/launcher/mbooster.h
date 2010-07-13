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
    MBooster();

    //! \brief Destructor
    virtual ~MBooster();

    //! \reimp
    virtual bool preload();

    /*!
     * \brief Return the socket name common to all MBooster objects.
     * \return Path to the socket file.
     */
    static const string & socketName();

    //! \reimp
    virtual char boosterType() const { return type(); }

    /*!
     * \brief Return a unique character ('d') represtenting the type of MBoosters.
     * \return Type character.
     */
    static char type();

protected:

    //! \reimp
    virtual const string & socketId() const;

private:

    //! Disable copy-constructor
    MBooster(const MBooster & r);

    //! Disable assignment operator
    MBooster & operator= (const MBooster & r);

    static const string m_socketId;

#ifdef UNIT_TEST
    friend class Ut_MBooster;
#endif
};

#endif // MBOOSTER_H
