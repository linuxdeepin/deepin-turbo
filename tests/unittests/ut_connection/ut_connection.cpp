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
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

/* redefine some methods for Connection class */
class MyConnection : public Connection
{
public:
    int nextMsg;
    char* nextStr;

    MyConnection(const string socketId);
    bool acceptConn();

private:
    bool  recvMsg(uint32_t *msg);
    const char * recvStr();
    bool  sendMsg(uint32_t msg);
    bool  sendStr(const char * str);
};

bool MyConnection::acceptConn() { return true; }

MyConnection::MyConnection(const string socketId) : 
    Connection(socketId), 
    nextMsg(0),
    nextStr(NULL)
{}

bool MyConnection::recvMsg(uint32_t *msg)
{
    *msg = nextMsg;
    return true;
}

bool MyConnection::sendMsg(uint32_t)
{ 
    return true;
}

bool MyConnection::sendStr(const char *)
{ 
    return true;
}

const char * MyConnection::recvStr()
{
    return nextStr;
}

Ut_Connection::Ut_Connection()
{
}

Ut_Connection::~Ut_Connection()
{
}

void Ut_Connection::initTestCase()
{
}

void Ut_Connection::cleanupTestCase()
{
}

/*
 * Check that socket initialized for provided socket id
 */
void Ut_Connection::testInitConnection()
{
    unsigned int prevNum = Connection::socketPool.size();
    Connection::initSocket("aaa");
    Connection::initSocket("bbb");
    Connection::initSocket("bbb");
    Connection::initSocket("aaa");

    QVERIFY(Connection::socketPool.size() == prevNum + 2);
    QVERIFY(Connection::findSocket("aaa") != -1);
    QVERIFY(Connection::findSocket("ccc") == -1);
    QVERIFY(Connection::findSocket("bbb") != -1);

    unlink("aaa");
    unlink("bbb");
}

/* 
 * Check that closeConn() reset socket connection
 */
void Ut_Connection::testAcceptConnection()
{
    char* socketName = (char*) "testAccept";

    Connection::initSocket(socketName);
    MyConnection* conn = new MyConnection(socketName);
    conn->m_fd = 1000;

    QVERIFY(conn->acceptConn() == true);
    QVERIFY(conn->m_fd > 0);

    conn->closeConn();
    QVERIFY(conn->m_fd == -1);

    unlink("testAccept");
}

/* 
 * Check that env variable passed from invoker will 
 * be set in launcher process
 */
void Ut_Connection::testGetEnv()
{
    QVERIFY(getenv("MY_TEST_ENV_VAR") == NULL);
    QVERIFY(getenv("PATH") != NULL);

    const char* socketName = "testGetEnv";
    Connection::initSocket(socketName);
    MyConnection* conn = new MyConnection(socketName);

    char* envVar = strdup("MY_TEST_ENV_VAR=3");

    conn->nextMsg = 1;
    conn->nextStr = envVar; 

    QVERIFY(conn->receiveEnv() == true);
    QVERIFY(getenv("MY_TEST_ENV_VAR") != NULL);
    QVERIFY(getenv("PATH") != NULL);

    unlink(socketName);
    delete envVar;
}

/*
 * Check getAppName() function correctness
 */
void Ut_Connection::testGetAppName()
{
    const char* socketName = "testGetAppName";

    Connection::initSocket(socketName);

    MyConnection* conn = new MyConnection(socketName);

    // wrong type of message
    conn->nextMsg = INVOKER_MSG_EXEC;
    string wrongStr = conn->receiveAppName();
    QVERIFY(wrongStr.empty());

    // empty app name
    conn->nextMsg = INVOKER_MSG_NAME;
    conn->nextStr = NULL;
    string emptyName = conn->receiveAppName();
    QVERIFY(emptyName.empty());

    // real name
    string realName("looooongApplicationName");
    char* dupName = strdup(realName.c_str());

    conn->nextMsg = INVOKER_MSG_NAME;
    conn->nextStr = dupName;

    string resName = conn->receiveAppName();
    QVERIFY(!resName.empty());
    QVERIFY(resName.compare(realName) == 0);

    unlink(socketName);
}

QTEST_APPLESS_MAIN(Ut_Connection);
