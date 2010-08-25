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

#include "qtbooster.h"

const string QtBooster::m_socketId = "/tmp/qtlnchr";
int QtBooster::m_ProcessID = 0;

QtBooster::QtBooster()
{
}

QtBooster::~QtBooster()
{
}

const string & QtBooster::socketId() const
{
    return m_socketId;
}

const string & QtBooster::socketName()
{
    return m_socketId;
}

char QtBooster::type()
{
    return 'q';
}

bool QtBooster::preload()
{
    return true;
}

void QtBooster::setProcessId(int pid)
{
    m_ProcessID = pid;
}

int QtBooster::processId()
{
    return m_ProcessID;
}
