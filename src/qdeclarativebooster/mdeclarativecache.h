/*!
**
** @copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** @license GNU Lesser General Public License version 2.1
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

#ifndef MDECLARATIVECACHE_H
#define MDECLARATIVECACHE_H

#include <QString>

class MDeclarativeCachePrivate;
class QApplication;
class QDeclarativeView;

/*!
 * \class MDeclarativeCache.
 * \brief Cache class for QDeclarativeBooster.
 */
class MDeclarativeCache
{
public:

    //! Constructor.
    MDeclarativeCache() {};

    //! Destructor.
    virtual ~MDeclarativeCache() {};

    //! Populate cache with QApplication and QDeclarativeView
    static void populate();

    //! Returns QApplication instance from cache or creates a new one.
    /*!
     * Ownership of the returned object is passed to the caller.
     * NOTE: This is subject to change.
     */
    static QApplication *qApplication(int &argc, char **argv);

    //! Returns QDeclarativeView instance from cache or creates a new one.
    /*!
     * Ownership of the returned object is passed to the caller.
     * NOTE: This is subject to change.
     */
    static QDeclarativeView *qDeclarativeView();

    //! Returns the directory that contains the application executable.
    /*!
     * This function is deprecated. It used to be a workaround for QApplication::applicationDirPath()
     * not working on harmattan with qdeclarativebooster and Qt 4.7, but this has been fixed.
     */
    static QString applicationDirPath() __attribute__ ((deprecated));

    //! Returns the file path of the application executable.
    /*!
     * This function is deprecated. It used to be a workaround for QApplication::applicationFilePath()
     * not working on harmattan with qdeclarativebooster and Qt 4.7, but this has been fixed.
     */
    static QString applicationFilePath() __attribute__ ((deprecated));

protected:

    static MDeclarativeCachePrivate* const d_ptr;

private:

    //! Disable copy-constructor
    MDeclarativeCache(const MDeclarativeCache & r);

    //! Disable assignment operator
    MDeclarativeCache & operator= (const MDeclarativeCache & r);

#ifdef UNIT_TEST
    friend class Ut_MDeclarativeCache;
#endif
};

#endif //MDECLARATIVECACHE_H
