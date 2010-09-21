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

#ifndef MONITORBOOSTER_H
#define MONITORBOOSTER_H

#include <QObject>
#include <QStringList>
#include <QSharedPointer>
#include <MGConfItem>
#include "booster.h"


class QString;

/*! \class MonitorBooster
 *
 * MonitorBooster kills certain boosters e.g. when themeing or language changes.
 * Daemon will then restart the boosters.
 */
class MonitorBooster : public QObject,  public Booster
{
    Q_OBJECT

 public:

    //! Constructor.
    MonitorBooster();

    /*!
     * \brief Return the empty strung (not used in MonitorBooster).
     * \return empty string.
     */
    static const string & socketName();

    //! Add a GConf key to trigger booster process termination
    void addKey(const QString & key);

    //! Add a booster process name to be killed
    void addProcessName(const QString & processName);

    /*! Starts the killer. This will initialize a QCoreApplication, does
     *  not return.
     */
    void start();

    /*!
     * \brief Return a unique character ('k') represtenting the type of MonitorBooster.
     * \return Type character.
     */
    static char type();

    /*!
     * \brief Override default behaviour, don't wait for commands from invoker.
     * \return true on success
     */
    virtual bool readCommand();

    //! \reimp
    virtual char boosterType() const { return type(); }

    /*!
     * \brief Keep booster pid, should be reset before booster run application's main() function
     */
    static void setProcessId(int pid);

    /*!
     * \brief Return booster pid
     */
    static int processId();

    //! Return the artificial process name
    static const string & temporaryProcessName();

    //! \reimp
    virtual const string & boosterTemporaryProcessName() const;

protected:

    //! \reimp
    virtual const string & socketId() const;

private Q_SLOTS:

    //! Kill all added processes
    void killProcesses();
    
 private:

    static const string m_socketId;
    static int m_ProcessID;

    //! Process name to be used for booster
    static const string m_temporaryProcessName;

    QStringList m_processNames;
    QList<QSharedPointer<MGConfItem> > m_gConfItems;
};

#endif // MONITORBOOSTER_H
