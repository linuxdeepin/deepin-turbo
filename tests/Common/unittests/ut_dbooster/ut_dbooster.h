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

#ifndef UT_DBOOSTER_H
#define UT_DBOOSTER_H

#include<QtTest/QtTest>
#include<QObject>

#include <tr1/memory>

#define UNIT_TEST

class QDeclarativeBooster;

class Ut_DBooster : public QObject
{
    Q_OBJECT

public:
    Ut_DBooster();
    virtual ~Ut_DBooster();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testSocketName();
    void testType();
    void testPreload();

private:
    std::tr1::shared_ptr<QDeclarativeBooster> m_subject;
};

#endif // UT_DBOOSTER_H
