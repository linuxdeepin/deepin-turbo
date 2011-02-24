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

#include "ebooster.h"
#include "logger.h"

#include <errno.h>

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

const string EBooster::m_socketId  = "/tmp/booste";
const string EBooster::m_temporaryProcessName = "booster-e";

const string & EBooster::socketId() const
{
    return m_socketId;
}

const string & EBooster::socketName()
{
    return m_socketId;
}

const string & EBooster::temporaryProcessName()
{
    return m_temporaryProcessName;
}

const string & EBooster::boosterTemporaryProcessName() const
{
    return temporaryProcessName();
}

char EBooster::type()
{
    return 'e';
}

bool EBooster::preload()
{
    return true;
}

int EBooster::launchProcess()
{
    Booster::setEnvironmentBeforeLaunch();

    // Ensure a NULL-terminated argv
    char ** dummyArgv = new char * [appData()->argc() + 1];
    const int i2 = appData()->argc();
    for (int i = 0; i < i2; i++)
        dummyArgv[i] = strdup(appData()->argv()[i]);
    dummyArgv[i2] = NULL;

    // Exec the binary (execv returns only in case of an error).
    if (execv(appData()->fileName().c_str(),
              dummyArgv))
    {
        Logger::logError("EBooster: Couldn't execv '%s': %s",
                         appData()->fileName().c_str(),
                         strerror(errno));
    }

    // Delete dummy argv if execv failed
    for (int i = 0; i < i2; i++)
        delete dummyArgv[i];

    return EXIT_FAILURE;
}
