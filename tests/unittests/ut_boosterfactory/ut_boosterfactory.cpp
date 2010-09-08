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

#include "ut_boosterfactory.h"
#include "boosterfactory.h"
#include "booster.h"
#include "mbooster.h"
#include "qtbooster.h"
#include "wrtbooster.h"

Ut_BoosterFactory::Ut_BoosterFactory()
{}

Ut_BoosterFactory::~Ut_BoosterFactory()
{}

void Ut_BoosterFactory::initTestCase()
{
}

void Ut_BoosterFactory::cleanupTestCase()
{}

void Ut_BoosterFactory::testCreate()
{
    Booster * booster = BoosterFactory::create('q');
    QVERIFY(dynamic_cast<QtBooster *>(booster));
    delete booster;

    booster = BoosterFactory::create('m');
    QVERIFY(dynamic_cast<MBooster *>(booster));
    delete booster;

    booster = BoosterFactory::create('w');
    QVERIFY(dynamic_cast<WRTBooster *>(booster));
    delete booster;

    for (int i = 0; i < 256; i++)
    {
        unsigned char t = static_cast<unsigned char>(i);
        if (t != 'q' && t != 'm' && t != 'w')
        {
            QVERIFY(!BoosterFactory::create(t));
        }
    }
}

void Ut_BoosterFactory::testSetProcessIdToBooster()
{
    BoosterFactory::setProcessIdToBooster('q', 123);
    QVERIFY(QtBooster::processId() == 123);

    BoosterFactory::setProcessIdToBooster('m', 234);
    QVERIFY(MBooster::processId() == 234);

    BoosterFactory::setProcessIdToBooster('w', 345);
    QVERIFY(WRTBooster::processId() == 345);
}

void Ut_BoosterFactory::testGetBoosterTypeForPid()
{
    BoosterFactory::setProcessIdToBooster('q', 1);
    BoosterFactory::setProcessIdToBooster('m', 2);
    BoosterFactory::setProcessIdToBooster('w', 3);

    QVERIFY(BoosterFactory::getBoosterTypeForPid(1) == 'q');
    QVERIFY(BoosterFactory::getBoosterTypeForPid(2) == 'm');
    QVERIFY(BoosterFactory::getBoosterTypeForPid(3) == 'w');

    QVERIFY(BoosterFactory::getBoosterTypeForPid(0) == 0);

    for (int i = 0; i < 256; i++)
    {
        unsigned char t = static_cast<unsigned char>(i);
        if (t != 'q' && t != 'm' && t != 'w')
        {
            BoosterFactory::setProcessIdToBooster(t, 1000);
            QVERIFY(BoosterFactory::getBoosterTypeForPid(1000) == 0);
        }
    }
}

QTEST_APPLESS_MAIN(Ut_BoosterFactory);

