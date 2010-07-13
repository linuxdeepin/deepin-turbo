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

#include "ut_qtbooster.h"
#include "qtbooster.h"

Ut_QtBooster::Ut_QtBooster() :
    m_subject(new QtBooster)
{}

Ut_QtBooster::~Ut_QtBooster()
{}

void Ut_QtBooster::initTestCase()
{}

void Ut_QtBooster::cleanupTestCase()
{}

void Ut_QtBooster::testSocketName()
{
    QVERIFY2(QtBooster::socketName() == QtBooster::m_socketId, "Failure");
    QVERIFY2(m_subject->socketId() == QtBooster::m_socketId, "Failure");
}

void Ut_QtBooster::testType()
{
    QVERIFY2(QtBooster::type() == 'q', "Failure");
    QVERIFY2(m_subject->boosterType() == 'q', "Failure");
}

QTEST_APPLESS_MAIN(Ut_QtBooster);

