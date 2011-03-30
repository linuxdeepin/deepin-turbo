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

#ifndef UT_BOOSTER_H
#define UT_BOOSTER_H

#include<QtTest/QtTest>
#include<QObject>

#include <tr1/memory>

#define UNIT_TEST

class Booster;

class Ut_Booster : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testRenameProcess();
    void testRenameProcessNotEnoughSpace();
    void cleanupTestCase();

private:

    char ** packTwoArgs(const char * arg0, const char * arg1);
    std::tr1::shared_ptr<Booster> m_subject;
};

#endif // UT_BOOSTER_H
