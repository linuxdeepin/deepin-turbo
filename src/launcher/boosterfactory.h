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

#ifndef BOOSTERFACTORY_H
#define BOOSTERFACTORY_H

#include <sys/types.h>

class Booster;

/*!
 * \class BoosterFactory
 * \brief BoosterFactory creates a new booster of given type.
 */
class BoosterFactory
{
public:
    /*! \brief  Creates a new booster.
     *  \param  type Wanted booster type: 'q' = Qt, 'm' = MeeGo Touch, 'w' = WRT.
     *  \return Pointer to Booster on success, NULL if failed
     */
    static Booster * create(char type);

    /*! Set current process ID globally to the given booster type
     *  so that we know which booster to restart if on exits.
     */
    static void setProcessIdToBooster(char type, pid_t pid);

    /*! \brief Return the type of booster currently assigned to pid.
     *  \return 'q', 'm' or 'w' on success, 0 on failure.
     */
    static char getBoosterTypeForPid(pid_t pid);

    /*! \brief Return the pid of last created booster of particular type
     *  \param  type Wanted booster type: 'q' = Qt, 'm' = MeeGo Touch, 'w' = WRT
     *  \return booster pid
     */
    static pid_t getBoosterPidForType(char type);


private:
    BoosterFactory();
};

#endif // BOOSTERFACTORY_H
