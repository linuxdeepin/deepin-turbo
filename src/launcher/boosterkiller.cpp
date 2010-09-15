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

#include <QString>
#include <QCoreApplication>
#include <QObject>
#include <MGConfItem>
#include "boosterkiller.h"

#include <cstdlib>

void BoosterKiller::addKey(const QString & key)
{
    MGConfItem *item = new MGConfItem(key, 0);
    QObject::connect(item, SIGNAL(valueChanged()),
                     this, SLOT(killProcesses()));
}

void BoosterKiller::addProcessName(const QString & processName)
{
    processNames << processName;
}

void BoosterKiller::start()
{
    int argc = 0;
    //char **argv = 0;
    QCoreApplication(argc, 0).exec();
}

void BoosterKiller::killProcesses()
{
    Q_FOREACH(QString processName, processNames) {
        system( (QString("pkill ") + processName).toStdString().c_str() );
    }
}
