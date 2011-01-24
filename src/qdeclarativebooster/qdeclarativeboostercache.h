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

#ifndef QDECLARATIVEBOOSTERCACHE_H
#define QDECLARATIVEBOOSTERCACHE_H

class QDeclarativeBoosterCachePrivate;
class QApplication;
class QDeclarativeView;

/*!
 * \class QDeclarativeBoosterCache.
 * \brief Cache class for QDeclarativeBooster.
 */
class QDeclarativeBoosterCache
{
public:

    //! Constructor.
    QDeclarativeBoosterCache() {};

    //! Destructor.
    virtual ~QDeclarativeBoosterCache() {};

    //! Populate cache with QApplication and QDeclarativeView
    static void populate();

    //! Returns QApplication instance from cache or creates a new one.
    /*!
     * Ownership of the returned object is passed to the caller.
     */
    static QApplication *qApplication(int &argc, char **argv);

    //! Returns QDeclarativeView instance from cache or creates a new one.
    /*!
     * Ownership of the returned object is passed to the caller.
     */
    static QDeclarativeView *qDeclarativeView();

 protected:

    static QDeclarativeBoosterCachePrivate* const d_ptr;

private:

    //! Disable copy-constructor
    QDeclarativeBoosterCache(const QDeclarativeBoosterCache & r);

    //! Disable assignment operator
    QDeclarativeBoosterCache & operator= (const QDeclarativeBoosterCache & r);


#ifdef UNIT_TEST
    friend class Ut_QDeclarativeBoosterCache;
#endif
};

#endif //QDECLARATIVEBOOSTERCACHE_H
