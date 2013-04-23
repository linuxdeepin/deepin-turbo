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
#include <stdexcept>
#include <QString>

// Booster is an abstract base-class, so let's inherit it and
// define methods that are pure virtual
class MyBooster : public Booster
{
public:
    MyBooster();
    const std::string & boosterType() const;
    const std::string & boosterTemporaryProcessName() const;

protected:
    bool preload();

private:
    const string m_boosterType;
    const string m_temporaryProcessName;
};

MyBooster::MyBooster() :
    m_boosterType("TestBooster"),
    m_temporaryProcessName("test booster")
{}

const std::string & MyBooster::boosterType() const
{
    return m_boosterType;
}

bool MyBooster::preload() 
{
    return true;
}

const std::string & MyBooster::boosterTemporaryProcessName() const
{
    return m_temporaryProcessName;
}

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

void Ut_Booster::testRenameProcess()
{
    m_subject.reset(new MyBooster);

    // If appData()->appName isn't initialized, new process name is booster_x
    // 20 chars dummy buffer used to fool ps to show correct process name with args
    const int INIT_ARGC = 2;
    char ** initialArgv = packTwoArgs("oldName", "                    ");

    // Rename process
    const char * tempArgv[] = {m_subject->boosterTemporaryProcessName().c_str()};
    m_subject->renameProcess(INIT_ARGC, initialArgv, 1, tempArgv);

    // Check that new name and arguments fit and are correct
    QVERIFY(strcmp(initialArgv[0], m_subject->boosterTemporaryProcessName().c_str()) == 0);

    // Define and copy args because it's assumed that they are allocated in the heap
    // (AppData deletes the argv on exit)
    const int ARGC = 3;
    m_subject->appData()->setArgc(ARGC);

    char ** argv = new char * [ARGC];
    argv[0] = strdup("newName");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->appData()->setArgv(const_cast<const char **>(argv));
    m_subject->renameProcess(INIT_ARGC, initialArgv, ARGC, const_cast<const char **>(argv));

    // New name and arguments fit and are correct
    const char * result = "newName\0--foo\0--bar\0";
    QVERIFY2(memcmp(initialArgv[0], result, 20) == 0, initialArgv[0]);

    delete initialArgv[0];
    delete [] initialArgv;
}

void Ut_Booster::testRenameProcessNotEnoughSpace()
{
    m_subject.reset(new MyBooster);

    const int NEW_ARGC = 3;
    m_subject->appData()->setArgc(NEW_ARGC);
    char ** argv = new char * [NEW_ARGC];
    argv[0] = strdup("newNameLong");
    argv[1] = strdup("--foo");
    argv[2] = strdup("--bar");
    m_subject->appData()->setArgv(const_cast<const char **>(argv));

    const int INIT_ARGC = 2;
    char ** initialArgv = packTwoArgs("old0123", "");
    int initLen = strlen(initialArgv[0]) + 1 + strlen("");
    m_subject->renameProcess(INIT_ARGC, initialArgv,
                             m_subject->appData()->argc(), m_subject->appData()->argv());

    // Not enough space for the new name nor the arguments:
    // name should be cut
    QVERIFY(memcmp(initialArgv[0], m_subject->appData()->argv()[0], initLen) == 0);
    QVERIFY(initialArgv[0][initLen] == '\0');
    QVERIFY(initialArgv[0][initLen - 1] == 'L');
}

void Ut_Booster::testBoosterLauncherSocket()
{
    int testValue = 11;
    m_subject->setBoosterLauncherSocket(testValue);
    int resultValue = m_subject->boosterLauncherSocket();
    QVERIFY(resultValue == testValue);
}

void Ut_Booster::testLoadMain()
{

    m_subject->appData()->setFileName("/no/such/file");
    bool exceptionTriggered = false;
    QString exceptionDetails;
    try{
        m_subject->loadMain();
    }
    catch (std::runtime_error & e)
    {
        exceptionTriggered = true;
        exceptionDetails = e.what();
    }

    QVERIFY(exceptionTriggered == true);
    QVERIFY(exceptionDetails.contains("Booster: Loading invoked application failed:", Qt::CaseInsensitive) == true);

    m_subject->appData()->setFileName("/usr/share/applauncherd-tests/libutplugin.so");

    exceptionTriggered = false;
    exceptionDetails = "";
    try{
        m_subject->loadMain();
    }
    catch (std::runtime_error & e)
    {
        exceptionTriggered = true;
        exceptionDetails = e.what();
    }

    QVERIFY(exceptionTriggered == true);
    QVERIFY(exceptionDetails.contains("Booster: Loading symbol 'main' failed:", Qt::CaseInsensitive) == true) ;
}

QTEST_APPLESS_MAIN(Ut_Booster);

