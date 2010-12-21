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

#include "ut_connection.h"
#include "connection.h"
#include "socketmanager.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

// Redefine some methods for Connection class
class MyConnection : public Connection
{
public:
    int nextMsg;
    char* nextStr;

    MyConnection(int socketFd, bool testMode);

private:
    bool recvMsg(uint32_t *msg);
    const char* recvStr();
};

MyConnection::MyConnection(int socketFd, bool testMode) :
    Connection(socketFd, testMode),
    nextMsg(0),
    nextStr(NULL)
{}

bool MyConnection::recvMsg(uint32_t *msg)
{
    *msg = nextMsg;
    return true;
}

const char* MyConnection::recvStr()
{
    return nextStr;
}

Ut_Connection::Ut_Connection()
{}

Ut_Connection::~Ut_Connection()
{}

void Ut_Connection::initTestCase()
{}

void Ut_Connection::cleanupTestCase()
{}

/* 
 * Test that socket creation / closing works.
 */
void Ut_Connection::testSocket()
{
    const char* socketName = "testAccept";
    SocketManager sm;

    sm.initSocket(socketName);
    m_subject.reset(new MyConnection(sm.findSocket(socketName), false));
    m_subject->m_fd = 1000;

    QVERIFY(m_subject->m_fd > 0);

    m_subject->close();
    QVERIFY(m_subject->m_fd == -1);

    unlink(socketName);
}

/* 
 * Check that env variable passed from invoker will 
 * be set in launcher process.
 *
 * Run in the test mode (no sockets really created).
 */
void Ut_Connection::testGetEnv()
{
    QVERIFY(getenv("MY_TEST_ENV_VAR") == NULL);
    QVERIFY(getenv("PATH") != NULL);

    const int socketFd = 0;
    m_subject.reset(new MyConnection(socketFd, true));

    char* envVar = strdup("MY_TEST_ENV_VAR=3");

    m_subject->nextMsg = 1;
    m_subject->nextStr = envVar;

    QVERIFY(m_subject->receiveEnv() == true);
    QVERIFY(getenv("MY_TEST_ENV_VAR") != NULL);
    QVERIFY(getenv("PATH") != NULL);

    delete envVar;
}

/*
 * Check getAppName() function correctness
 *
 * Run in the test mode (no sockets really created).
 */
void Ut_Connection::testGetAppName()
{
    const int socketFd = 0;
    m_subject.reset(new MyConnection(socketFd, true));

    // Wrong type of message
    m_subject->nextMsg = INVOKER_MSG_EXEC;
    string wrongStr = m_subject->receiveAppName();
    QVERIFY(wrongStr.empty());

    // Empty app name
    m_subject->nextMsg = INVOKER_MSG_NAME;
    m_subject->nextStr = NULL;
    string emptyName = m_subject->receiveAppName();
    QVERIFY(emptyName.empty());

    // Real name
    string realName("looooongApplicationName");
    char* dupName = strdup(realName.c_str());

    m_subject->nextMsg = INVOKER_MSG_NAME;
    m_subject->nextStr = dupName;

    string resName = m_subject->receiveAppName();
    QVERIFY(!resName.empty());
    QVERIFY(resName.compare(realName) == 0);
}

QTEST_APPLESS_MAIN(Ut_Connection);
