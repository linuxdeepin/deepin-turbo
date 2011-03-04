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

#include "ut_dbooster.h"
#include "qdeclarativebooster.h"
#include "mdeclarativecache.h"

Ut_DBooster::Ut_DBooster() :
    m_subject(new QDeclarativeBooster)
{}

Ut_DBooster::~Ut_DBooster()
{}

void Ut_DBooster::initTestCase()
{}

void Ut_DBooster::cleanupTestCase()
{}

void Ut_DBooster::testSocketName()
{
    QVERIFY(QDeclarativeBooster::socketName() == QDeclarativeBooster::m_socketId);
    QVERIFY(m_subject->socketId() == QDeclarativeBooster::m_socketId);
}

void Ut_DBooster::testType()
{
    QVERIFY(QDeclarativeBooster::type() == 'd');
    QVERIFY(m_subject->boosterType() == 'd');
}

void Ut_DBooster::testPreload()
{
    m_subject->preload();

    const char * argv[] = {"foo"};
    int argc = 1;

    // TODO: Somehow make sure that MDeclarativeCache really returns the cached
    //       QApplication instead of creating a new one here.
    QApplication * app = MDeclarativeCache::qApplication(argc, const_cast<char **>(argv));
    QVERIFY(app);

    QStringList args = app->arguments();
    QVERIFY(args.length() == 1);
    QVERIFY(args[0] == "foo");

    QVERIFY(MDeclarativeCache::qDeclarativeView());
}

QTEST_APPLESS_MAIN(Ut_DBooster);

