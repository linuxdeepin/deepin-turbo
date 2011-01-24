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

#include "qdeclarativebooster.h"
#include "qdeclarativeboostercache.h"

const string QDeclarativeBooster::m_socketId = "/tmp/boostd";
const string QDeclarativeBooster::m_temporaryProcessName = "booster-d";

const string & QDeclarativeBooster::socketId() const
{
    return m_socketId;
}

const string & QDeclarativeBooster::socketName()
{
    return m_socketId;
}

const string & QDeclarativeBooster::temporaryProcessName()
{
    return m_temporaryProcessName;
}

const string & QDeclarativeBooster::boosterTemporaryProcessName() const
{
    return temporaryProcessName();
}

char QDeclarativeBooster::type()
{
    return 'd';
}

bool QDeclarativeBooster::preload()
{
    QDeclarativeBoosterCache::populate();
    return true;
}
