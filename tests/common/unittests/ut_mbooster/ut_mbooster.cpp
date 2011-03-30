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

#include "ut_mbooster.h"
#include "mbooster.h"

#ifdef HAVE_MCOMPONENTCACHE
#include <MComponentCache>
#endif

Ut_MBooster::Ut_MBooster() :
    m_subject(new MBooster)
{}

Ut_MBooster::~Ut_MBooster()
{}

void Ut_MBooster::initTestCase()
{}

void Ut_MBooster::cleanupTestCase()
{}

void Ut_MBooster::testSocketName()
{
    QVERIFY(MBooster::socketName() == MBooster::m_socketId);
    QVERIFY(m_subject->socketId() == MBooster::m_socketId);
}

void Ut_MBooster::testType()
{
    QVERIFY(MBooster::type() == 'm');
    QVERIFY(m_subject->boosterType() == 'm');
}

void Ut_MBooster::testPreload()
{
#ifdef HAVE_MCOMPONENTCACHE

    m_subject->preload();

    const char * argv[] = {"foo"};
    int argc = 1;

    QVERIFY(MComponentCache::mApplication(argc, const_cast<char **>(argv)));
    QVERIFY(MComponentCache::mApplicationWindow());

#endif
}

QTEST_APPLESS_MAIN(Ut_MBooster);

