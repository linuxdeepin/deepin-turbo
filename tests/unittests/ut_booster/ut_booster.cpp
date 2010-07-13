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

#include "ut_booster.h"
#include "booster.h"

// Booster is an abstract base-class, so let's inherit it
class MyBooster : public Booster
{
public:
    MyBooster();
    char boosterType() const;
    const std::string & socketId() const;

private:
    const string m_socketId;
};

MyBooster::MyBooster() :
    m_socketId("/tmp/MyBooster")
{}

char MyBooster::boosterType() const
{
    return 'm';
}

const std::string & MyBooster::socketId() const
{
    return m_socketId;
}

Ut_Booster::Ut_Booster()
{}

Ut_Booster::~Ut_Booster()
{}

void Ut_Booster::initTestCase()
{}

void Ut_Booster::cleanupTestCase()
{}

char ** Ut_Booster::packTwoArgs(const char * arg0, const char * arg1)
{
    char ** argv  = new char * [2];
    char * result = new char[strlen(arg0) + strlen(arg1) + 2];
    memset(result, '\0', strlen(arg0) + strlen(arg1) + 2);

    strcat(result, arg0);
    strcat(result, " ");
    strcat(result, arg1);

    // Arguments are allocated consecutively in Linux
    argv[0] = result;
    argv[1] = argv[0] + strlen(arg0) + 1;
    argv[0][strlen(arg0)] = '\0';

    return argv;
}

void Ut_Booster::testRenameBoosterProcess()
{
    m_subject.reset(new MyBooster);

    // if m_app.appName isn't initialized, new process name is booster_x
    // 20 chars dummy buffer used to fool ps to show correct process name with args
    const int INIT_ARGS = 2;
    char ** initialArgv = packTwoArgs("oldName", "                    ");
    m_subject->renameProcess(INIT_ARGS, const_cast<char **>(initialArgv));

    // New name and arguments fit and are correct
    QVERIFY(strcmp(initialArgv[0], "booster-m") == 0);

    // Define and copy args because it's assumed that they are allocated in the heap
    // (AppData deletes the argv on exit)
    const int ARGS = 3;
    m_subject->m_app.setArgc(ARGS);

    char ** argv = new char * [ARGS];
    argv[0] = strdup("newName");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->m_app.setArgv(const_cast<const char **>(argv));
    m_subject->m_app.setAppName("newName");
    m_subject->renameProcess(INIT_ARGS, const_cast<char **>(initialArgv));

    // New name and arguments fit and are correct
    QVERIFY2(strcmp(initialArgv[0], "newName --foo --bar") == 0, initialArgv[0]);

    delete initialArgv[0];
    delete [] initialArgv;
}

void Ut_Booster::testRenameProcess()
{
    m_subject.reset(new MyBooster);

    // Define and copy args because it's assumed that they are allocated in the heap
    // (AppData deletes the argv on exit)
    const int ARGS = 3;
    m_subject->m_app.setArgc(ARGS);
    char ** argv = new char * [ARGS]; 
    argv[0] = strdup("newName");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->m_app.setArgv(const_cast<const char **>(argv));
    m_subject->m_app.setAppName(m_subject->m_app.argv()[0]);

    // 20 chars dummy buffer used to fool ps to show correct process name with args
    const int INIT_ARGS = 2;
    char ** initialArgv = packTwoArgs("oldName", "                    ");
    m_subject->renameProcess(INIT_ARGS, const_cast<char **>(initialArgv));

    // New name and arguments fit and are correct
    QVERIFY(strcmp(initialArgv[0], "newName --foo --bar") == 0);

    delete initialArgv[0];
    delete [] initialArgv;
}

void Ut_Booster::testRenameProcessNotEnoughSpace()
{
    m_subject.reset(new MyBooster);

    const int ARGS = 3;
    m_subject->m_app.setArgc(ARGS);
    char ** argv = new char * [ARGS]; 
    argv[0] = strdup("newNameLong");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->m_app.setArgv(const_cast<const char **>(argv));
    m_subject->m_app.setAppName(m_subject->m_app.argv()[0]);

    const int INIT_ARGS = 2;
    char ** initialArgv = packTwoArgs("oldName", "   ");
    int initLen = strlen(initialArgv[0]);
    m_subject->renameProcess(INIT_ARGS, initialArgv);

    // Not enough space for the new name nor the arguments:
    // name should be cut
    QVERIFY(strncmp(initialArgv[0], m_subject->m_app.argv()[0], initLen - 1) == 0);

    delete [] initialArgv[0];
    delete [] initialArgv;
}

void Ut_Booster::testRenameProcessNotEnoughSpace2()
{
    m_subject.reset(new MyBooster);

    const int ARGS = 3;
    m_subject->m_app.setArgc(ARGS);
    char ** argv = new char * [ARGS]; 
    argv[0] = strdup("newName");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->m_app.setArgv(const_cast<const char **>(argv));
    m_subject->m_app.setAppName(m_subject->m_app.argv()[0]);

    const int INIT_ARGS = 2;
    char ** initialArgv = packTwoArgs("oldName", "        ");
    m_subject->renameProcess(INIT_ARGS, initialArgv);

    // Not enough space for the second argument:
    // second argument should be left out
    QVERIFY(strcmp(initialArgv[0], "newName --foo") == 0);

    delete initialArgv[0];
    delete [] initialArgv;
}

void Ut_Booster::testRenameProcessNotEnoughSpace3()
{
    m_subject.reset(new MyBooster);

    const int ARGS = 3;
    m_subject->m_app.setArgc(ARGS);
    char ** argv = new char * [ARGS]; 
    argv[0] = strdup("newName");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->m_app.setArgv(const_cast<const char **>(argv));
    m_subject->m_app.setAppName(m_subject->m_app.argv()[0]);

    const int INIT_ARGS = 2;
    char ** initialArgv = packTwoArgs("app", "    ");

    m_subject->renameProcess(INIT_ARGS, initialArgv);

    // Not enough space for arguments but just enough space
    // for the new name
    QVERIFY(strcmp(initialArgv[0], "newName") == 0);

    delete initialArgv[0];
    delete [] initialArgv;
}

void Ut_Booster::testRenameProcessNotEnoughSpace4()
{
    m_subject.reset(new MyBooster);

    const int ARGS = 3;
    m_subject->m_app.setArgc(ARGS);
    char ** argv = new char * [ARGS]; 
    argv[0] = strdup("newNameLongLong");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->m_app.setArgv(const_cast<const char **>(argv));
    m_subject->m_app.setAppName(m_subject->m_app.argv()[0]);

    const int INIT_ARGS = 2;
    char ** initialArgv = packTwoArgs("app", "   ");
    m_subject->renameProcess(INIT_ARGS, initialArgv);

    // Not enough space for newName, but dummy space exist: should be cut
    QVERIFY(strcmp(initialArgv[0], "newName") == 0);

    delete initialArgv[0];
    delete [] initialArgv;
}

void Ut_Booster::testRenameProcessNoArgs()
{
    m_subject.reset(new MyBooster);

    const int ARGS = 2;
    m_subject->m_app.setArgc(ARGS);
    char ** argv = new char * [ARGS]; 
    argv[0] = strdup("newName");
    argv[1] = strdup("--foo");
    m_subject->m_app.setArgv(const_cast<const char **>(argv));
    m_subject->m_app.setAppName(m_subject->m_app.argv()[0]);

    const int INIT_ARGS = 1;
    char ** initialArgv = new char * [INIT_ARGS];
    initialArgv[0]      = strdup("oldName");
    m_subject->renameProcess(INIT_ARGS, initialArgv);

    // No dummy space argument at all, only name fits
    QVERIFY(strcmp(initialArgv[0], m_subject->m_app.argv()[0]) == 0);

    delete initialArgv[0];
    delete [] initialArgv;
}

QTEST_APPLESS_MAIN(Ut_Booster);

