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

#include "mbooster.h"
#include "logger.h"

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

const string MBooster::m_socketId  = "/tmp/mlnchr";
int MBooster::m_ProcessID = 0;

MBooster::MBooster()
{
}

MBooster::~MBooster()
{
}

const string & MBooster::socketId() const
{
    return m_socketId;
}

bool MBooster::preload()
{
#ifdef HAVE_MCOMPONENTCACHE
    MComponentCache::populateForMApplication();
#endif
    return true;
}

const string & MBooster::socketName()
{
    return m_socketId;
}

char MBooster::type()
{
    return 'm';
}

void MBooster::setProcessId(int pid)
{
    m_ProcessID = pid;
}


int MBooster::ProcessId()
{
    return m_ProcessID;
}
