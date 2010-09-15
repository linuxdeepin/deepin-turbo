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

#ifndef BOOSTERKILLER_H
#define BOOSTERKILLER_H

#include <QObject>
#include <QStringList>

class QString;
class MGConfItem;

class BoosterKiller: public QObject
{
    Q_OBJECT

 public:
    /* Add a GConf key and the name of the process that should be
       killed in case the value associated to the key is changed.
    */
    void addKey(const QString & key);
    void addProcessName(const QString & processName);

    /* Starts the killer. This will initialize qcoreapplication, does
       not return.
     */
    void start();

 private slots:
    void killProcesses();
    
 private:
    QList<MGConfItem*> gconfItems;
    QStringList processNames;
    
};

#endif
