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

#include "ut_wrtbooster.h"
#include "wrtbooster.h"

#ifdef HAVE_MCOMPONENTCACHE
#include <MComponentCache>
#endif

Ut_WRTBooster::Ut_WRTBooster() :
    m_subject(new WRTBooster)
{}

Ut_WRTBooster::~Ut_WRTBooster()
{}

void Ut_WRTBooster::initTestCase()
{}

void Ut_WRTBooster::cleanupTestCase()
{}

void Ut_WRTBooster::testSocketName()
{
    QVERIFY(WRTBooster::socketName() == WRTBooster::m_socketId);
    QVERIFY(m_subject->socketId() == WRTBooster::m_socketId);
}

void Ut_WRTBooster::testType()
{
    QVERIFY(WRTBooster::type() == 'w');
    QVERIFY(m_subject->boosterType() == 'w');
}

void Ut_WRTBooster::testSetProcessId()
{
    WRTBooster::setProcessId(123);
    QVERIFY(WRTBooster::processId() == 123);
}

void Ut_WRTBooster::testPreload()
{
#ifdef HAVE_MCOMPONENTCACHE

    m_subject->preload();

    const char * argv[] = {"foo"};
    int argc = 1;

    QVERIFY(MComponentCache::mApplication(argc, const_cast<char **>(argv)));
    QVERIFY(MComponentCache::mApplicationWindow());

#endif
}

QTEST_APPLESS_MAIN(Ut_WRTBooster);
