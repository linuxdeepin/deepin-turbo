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

#include "ut_socketmanager.h"
#include "socketmanager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

Ut_SocketManager::Ut_SocketManager()
{}

Ut_SocketManager::~Ut_SocketManager()
{}

void Ut_SocketManager::initTestCase()
{}

void Ut_SocketManager::cleanupTestCase()
{}

// Check that socket gets initialized for provided socket id
void Ut_SocketManager::testInitSocket()
{
    m_subject.reset(new SocketManager);

    QVERIFY(m_subject->socketCount() == 0);

    QString name1 = QString("a-%1").arg(getpid());
    QString name2 = QString("b-%1").arg(getpid());

    m_subject->initSocket(name1);
    m_subject->initSocket(name2);
    m_subject->initSocket(name2);
    m_subject->initSocket(name1);

    QVERIFY(m_subject->socketCount() == 2);

    QVERIFY(m_subject->findSocket(name1) != -1);
    QVERIFY(m_subject->findSocket(name2) != -1);
    QVERIFY(m_subject->findSocket("foo") == -1);

    QVERIFY(m_subject->socketCount() == 2);

    m_subject->closeAllSockets();

    QVERIFY(m_subject->socketCount() == 0);

    QVERIFY(unlink(name1.toStdString().c_str()) == 0);
    QVERIFY(unlink(name2.toStdString().c_str()) == 0);
}

QTEST_APPLESS_MAIN(Ut_SocketManager);
