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
    QVERIFY2(MBooster::socketName() == MBooster::m_socketId, "Failure");
    QVERIFY2(m_subject->socketId() == MBooster::m_socketId, "Failure");
}

void Ut_MBooster::testType()
{
    QVERIFY2(MBooster::type() == 'm', "Failure");
    QVERIFY2(m_subject->boosterType() == 'm', "Failure");
}

QTEST_APPLESS_MAIN(Ut_MBooster);

