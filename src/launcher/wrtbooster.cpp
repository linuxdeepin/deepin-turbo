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

#include "wrtbooster.h"
#include "logger.h"

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

const string WRTBooster::m_socketId  = "/tmp/boostw";
int WRTBooster::m_ProcessID = 0;

WRTBooster::WRTBooster()
{
}

WRTBooster::~WRTBooster()
{
}

const string & WRTBooster::socketId() const
{
    return m_socketId;
}

bool WRTBooster::preload()
{
#ifdef HAVE_MCOMPONENTCACHE
    MComponentCache::populateForWRTApplication();
#endif
    return true;
}

const string & WRTBooster::socketName()
{
    return m_socketId;
}

char WRTBooster::type()
{
    return 'w';
}

void WRTBooster::setProcessId(int pid)
{
    m_ProcessID = pid;
}


int WRTBooster::ProcessId()
{
    return m_ProcessID;
}
