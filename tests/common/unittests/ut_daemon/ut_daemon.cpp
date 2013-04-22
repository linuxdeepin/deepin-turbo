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
    int argc = 2;
    char **argv = new char * [argc];
    argv[0] = strdup("app");
    argv[1] = strdup("--boot-mode");

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

    QVERIFY(m_subject->m_initialArgc == 2);

    QCOMPARE(m_subject->m_initialArgv[0], "app");
    QCOMPARE(m_subject->m_initialArgv[1], "--boot-mode");

}

void Ut_Daemon::testParseArgs()
{
    int argc = 2;
    char **argv = new char * [argc];

    argv[0] = strdup("app");
    argv[1] = strdup("--daemon");

    QVERIFY(m_subject->m_daemon == false);

    m_subject->parseArgs(vector<string>(argv, argv + argc));

    QVERIFY(m_subject->m_daemon == true);

    delete argv[0];
    delete argv[1];
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

void Ut_Daemon::testForkBooster()
{
    //negative testcase for unregistered booster type '0'
    pid_t pid = fork();
    if (pid == 0) { // child
        // Code only executed by child process
        m_subject->forkBooster();
        QFAIL("Not exited on invalid booster type");
        _exit(0); //exit from child if something goes wrong
    } else if (pid < 0) { // failed to fork
        QFAIL("Unable to fork for test _exit");
    } else { //parent only
        QTest::qSleep(1000);
        int resultCode = QProcess::execute("grep",QStringList() << "Daemon: Unknown booster type '0'" << "/var/log/syslog");
        QVERIFY(resultCode == 0);
    }
}


void Ut_Daemon::testReadFromBoosterSocket()
{
    //negative testcase reads from invalid socket
    pid_t pid = fork();
    if (pid == 0) { // child
        // Code only executed by child process
        m_subject->readFromBoosterSocket(-1);
        QFAIL("Not exited on invalid socket");
        _exit(0); //exit from child if something goes wrong
    } else if (pid < 0) { // failed to fork
        QFAIL("Unable to fork for test _exit");
    } else { //parent only
        QTest::qSleep(1000);
        int resultCode = QProcess::execute("grep",QStringList() << "Daemon: Nothing read from the socket" << "/var/log/syslog");
        QVERIFY(resultCode == 0);
    }
}

QTEST_APPLESS_MAIN(Ut_Daemon)
