/***************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QDECLARATIVEBOOSTER_H
#define QDECLARATIVEBOOSTER_H

#include "booster.h"

/*!
 * \class QDeclarativeBooster.
 * \brief QDeclarative-specific version of the Booster.
 */
class QDeclarativeBooster : public Booster
{
public:

    //! Constructor.
    QDeclarativeBooster() {};

    //! Destructor.
    virtual ~QDeclarativeBooster() {};

    /*!
     * \brief Return the socket name common to all QDeclarativeBooster objects.
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
     * \brief Return a unique character ('d') represtenting the type of QDeclarativeBoosters.
     * \return Type character.
     */
    static char type();

    //! \reimp
    virtual bool preload();

protected:

    //! \reimp
    virtual const string & socketId() const;

private:

    //! Disable copy-constructor
    QDeclarativeBooster(const QDeclarativeBooster & r);

    //! Disable assignment operator
    QDeclarativeBooster & operator= (const QDeclarativeBooster & r);

    static const string m_socketId;

    //! Process name to be used when booster is not
    //! yet transformed into a running application
    static const string m_temporaryProcessName;

#ifdef UNIT_TEST
    friend class Ut_DBooster;
#endif
};

#endif //QDECLARATIVEBOOSTER_H
