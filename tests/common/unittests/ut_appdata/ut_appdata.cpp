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

#include "ut_appdata.h"
#include "appdata.h"


Ut_AppData::Ut_AppData() 
    
{}

Ut_AppData::~Ut_AppData()
{}

void Ut_AppData::initTestCase()
{}

void Ut_AppData::cleanupTestCase()
{}




void Ut_AppData::testEntry()
{
  /*    QVERIFY(!m_subject->pluginEntry()); //plugin not loaded yet

    void * handle = dlopen(SINGLE_INSTANCE_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(m_subject->validateAndRegisterPlugin(handle) == true);
    QVERIFY(m_subject->pluginEntry()); //plugin loaded
    m_subject->closePlugin(); 

void AppData::setEntry(entry_t newEntry)
{
    m_entry = newEntry;
}

entry_t AppData::entry() const
{
    return m_entry;
}

*/
AppData * appData = new AppData();
QVERIFY(appData);
entry_t newEntry = (entry_t)(new int(1)); 
appData->setEntry(newEntry);
entry_t checkEntry = appData->entry();
QVERIFY(checkEntry == newEntry);
delete appData;
appData = NULL; 
}


QTEST_APPLESS_MAIN(Ut_AppData);
