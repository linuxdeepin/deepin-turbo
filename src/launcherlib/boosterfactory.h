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

class Booster;

/*!
 * \class BoosterFactory
 * \brief BoosterFactory creates a new booster of given type.
 */
// TODO: Could be a namespace instead of a class.
class BoosterFactory
{
public:
    /*! \brief  Creates a new booster.
     *  \param  type Wanted booster type: 'q' = Qt, 'm' = MeeGo Touch, 'w' = WRT.
     *  \return Pointer to Booster on success, NULL if failed
     */
    static Booster * create(char type);

private:

    //! Hidden constructor
    BoosterFactory() {};
};

#endif // BOOSTERFACTORY_H
