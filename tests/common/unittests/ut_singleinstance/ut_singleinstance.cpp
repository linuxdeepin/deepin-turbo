/***************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "ut_singleinstance.h"
#include "singleinstance.h"
#include <dlfcn.h>

Ut_SingleInstance::Ut_SingleInstance() :
    m_subject(new SingleInstance)
{}

Ut_SingleInstance::~Ut_SingleInstance()
{}

void Ut_SingleInstance::initTestCase()
{}

void Ut_SingleInstance::cleanupTestCase()
{}

void Ut_SingleInstance::testValidateAndRegisterPlugin()
{
//positive
    void * handle = dlopen(SINGLE_INSTANCE_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(m_subject->validateAndRegisterPlugin(handle) == true);
    m_subject->closePlugin();
//negative invalid handle
    handle = 0;
    QVERIFY(m_subject->validateAndRegisterPlugin(handle) == false);
//negative no "unlock" symbol in the plugin (only lock)
    handle = dlopen(TEST_LIB_LOCK_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(m_subject->validateAndRegisterPlugin(handle) == false);
//negative no "activateExistingInstance" symbol in the plugin (only lock and unlock)
    handle = dlopen(TEST_LIB_LOCKUNLOCK_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(m_subject->validateAndRegisterPlugin(handle) == false);
}

void Ut_SingleInstance::testPluginEntry()
{
    QVERIFY(!m_subject->pluginEntry()); //plugin not loaded yet

    void * handle = dlopen(SINGLE_INSTANCE_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(m_subject->validateAndRegisterPlugin(handle) == true);
    QVERIFY(m_subject->pluginEntry()); //plugin loaded
    m_subject->closePlugin();
}


QTEST_APPLESS_MAIN(Ut_SingleInstance);
