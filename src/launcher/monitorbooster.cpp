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

const string MonitorBooster::m_socketId = "";
int MonitorBooster::m_ProcessID = 0;
const string MonitorBooster::m_temporaryProcessName = "booster-monitor";

MonitorBooster::MonitorBooster()
{
    // Add keys to listen to.
    addKey(MEEGOTOUCH_THEME_GCONF_KEY);
    addKey(MEEGOTOUCH_LANGUAGE_GCONF_KEY);

    // Add process names to be killed if the state of
    // added keys changes.
    addProcessName(MBooster::temporaryProcessName().c_str());
    addProcessName(WRTBooster::temporaryProcessName().c_str());
}

void MonitorBooster::addKey(const QString & key)
{
    MGConfItem * item = new MGConfItem(key, 0);
    m_gConfItems << QSharedPointer<MGConfItem>(item);

    QObject::connect(item, SIGNAL(valueChanged()),
                     this, SLOT(killProcesses()));
}

void MonitorBooster::addProcessName(const QString & processName)
{
    m_processNames << processName;
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

void MonitorBooster::killProcesses()
{
    Q_FOREACH(QString processName, m_processNames) {
        system( (QString("pkill ") + processName).toStdString().c_str() );
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
