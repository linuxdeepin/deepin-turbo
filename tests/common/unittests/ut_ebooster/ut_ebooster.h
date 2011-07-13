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

#ifndef UT_EBOOSTER_H
#define UT_EBOOSTER_H

#include<QtTest/QtTest>
#include<QObject>

#include <tr1/memory>

#define UNIT_TEST

class EBooster;

class Ut_EBooster : public QObject
{
    Q_OBJECT

public:
    Ut_EBooster();
    virtual ~Ut_EBooster();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testSocketName();
    void testType();
    void testPreload();
    void testTemporaryProcessName();
    void testLaunchProcessWithBadArg();

private:
    std::tr1::shared_ptr<EBooster> m_subject;
};

#endif // UT_EBOOSTER_H
