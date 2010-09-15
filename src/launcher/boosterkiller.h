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
#include <QSharedPointer>
#include <MGConfItem>

class QString;

/*! \class BoosterKiller
 *
 * BoosterKiller kills certain boosters e.g. when themeing or language changes.
 * Daemon will then restart the boosters.
 */
class BoosterKiller : public QObject
{
    Q_OBJECT

 public:

    //! Add a GConf key to trigger booster process termination
    void addKey(const QString & key);

    //! Add a booster process name to be killed
    void addProcessName(const QString & processName);

    /*! Starts the killer. This will initialize a QCoreApplication, does
     *  not return.
     */
    void start();

 private Q_SLOTS:

    //! Kill all added processes
    void killProcesses();
    
 private:

    QStringList m_processNames;
    QList<QSharedPointer<MGConfItem> > m_gConfItems;
};

#endif // BOOSTERKILLER_H
