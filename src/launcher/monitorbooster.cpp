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

#include <QCoreApplication>
#include <QObject>
#include <cstdlib>

#include "monitorbooster.h"
#include "mbooster.h"
#include "wrtbooster.h"
#include "logger.h"

const string MonitorBooster::m_socketId = "";
int MonitorBooster::m_ProcessID = 0;
const string MonitorBooster::m_temporaryProcessName = "booster-monitor";

MonitorBooster::MonitorBooster()
{
    // Add keys to listen to.
    addKey(MEEGOTOUCH_THEME_GCONF_KEY);
    addKey(MEEGOTOUCH_LANGUAGE_GCONF_KEY);
}

void MonitorBooster::addKey(const QString & key)
{
    MGConfItem * item = new MGConfItem(key, 0);
    m_gConfItems << QSharedPointer<MGConfItem>(item);

    QObject::connect(item, SIGNAL(valueChanged()),
                     this, SLOT(notifyKeyChange()));
}

void MonitorBooster::run()
{
    int argc = 0;
    QCoreApplication(argc, 0).exec();
}

void MonitorBooster::initialize(int initialArgc, char ** initialArgv, int newPipeFd[2])
{
    setPipeFd(newPipeFd);

    // Clean-up all the env variables
    clearenv();

    // Rename process to temporary booster process name
    renameProcess(initialArgc, initialArgv);
}

void MonitorBooster::notifyKeyChange()
{
    // Signal the parent process that it can create a new
    // waiting booster process and close write end
    const char msg = boosterType();
    ssize_t ret = write(pipeFd(1), reinterpret_cast<const void *>(&msg), 1);
    if (ret == -1) {
        Logger::logError("MonitorBooster: Couldn't send type message to launcher process\n");
    }
}

char MonitorBooster::type()
{
    return 'k';
}

const string & MonitorBooster::socketName()
{
    return m_socketId;
}

const string & MonitorBooster::socketId() const
{
    return m_socketId;
}

void MonitorBooster::setProcessId(int pid)
{
    m_ProcessID = pid;
}

int MonitorBooster::processId()
{
    return m_ProcessID;
}

const string & MonitorBooster::temporaryProcessName()
{
    return m_temporaryProcessName;
}

const string & MonitorBooster::boosterTemporaryProcessName() const
{
    return temporaryProcessName();
}

char MonitorBooster::boosterType() const
{
    return type();
}

