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

#include "ut_daemon.h"
#include "daemon.h"

Ut_Daemon::Ut_Daemon()
{
    int argc = 3;
    char **argv = new char * [argc];
    argv[0] = strdup("app");
    argv[1] = strdup("--testParameter");
    argv[2] = strdup("--123");

    m_subject.reset(new Daemon( argc, argv ));
}

Ut_Daemon::~Ut_Daemon()
{}

void Ut_Daemon::initTestCase()
{}

void Ut_Daemon::cleanupTestCase()
{}

void Ut_Daemon::testInitialArguments()
{
    QVERIFY(m_subject->m_initialArgc == 3);

    QCOMPARE(m_subject->m_initialArgv[0], "app");
    QCOMPARE(m_subject->m_initialArgv[1], "--testParameter");
    QCOMPARE(m_subject->m_initialArgv[2], "--123");
}

void Ut_Daemon::testParseArgs()
{
    int argc = 3;
    char **argv = new char * [argc];

    argv[0] = strdup("app");
    argv[1] = strdup("--daemon");
    argv[2] = strdup("--quiet");

    QVERIFY(m_subject->m_daemon == false);
    QVERIFY(m_subject->m_quiet == false);

    m_subject->parseArgs(vector<string>(argv, argv + argc));

    QVERIFY(m_subject->m_daemon == true);
    QVERIFY(m_subject->m_quiet == true);

    delete argv[0];
    delete argv[1];
    delete argv[2];
    delete [] argv;
}

void Ut_Daemon::testVerifyInstance()
{
    QVERIFY(m_subject.get() == Daemon::instance());
}

void Ut_Daemon::testReapZombies()
{
    QVERIFY(m_subject->m_children.size() == 0);

    for (int i = 1; i < 10; i++) {
        m_subject->m_children.push_back(i);
    }

    m_subject->reapZombies();

    QVERIFY(m_subject->m_children.size() == 0);
}

QTEST_APPLESS_MAIN(Ut_Daemon);
