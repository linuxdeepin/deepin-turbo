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

#include "ut_ebooster.h"
#include "ebooster.h"

Ut_EBooster::Ut_EBooster() :
    m_subject(new EBooster)
{}

Ut_EBooster::~Ut_EBooster()
{}

void Ut_EBooster::initTestCase()
{}

void Ut_EBooster::cleanupTestCase()
{}

void Ut_EBooster::testSocketName()
{
    QVERIFY(EBooster::socketName() == EBooster::m_socketId);
    QVERIFY(m_subject->socketId() == EBooster::m_socketId);
}

void Ut_EBooster::testType()
{
    QVERIFY(EBooster::type() == 'e');
    QVERIFY(m_subject->boosterType() == 'e');
}

void Ut_EBooster::testPreload()
{
    QVERIFY(m_subject->preload());
}

QTEST_APPLESS_MAIN(Ut_EBooster);
