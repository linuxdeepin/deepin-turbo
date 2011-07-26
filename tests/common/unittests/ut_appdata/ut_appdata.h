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

#ifndef UT_APPDATA_H
#define UT_APPDATA_H

#include<QtTest/QtTest>
#include<QObject>



#define UNIT_TEST

class Ut_AppData : public QObject
{
    Q_OBJECT

public:
    Ut_AppData();
    virtual ~Ut_AppData();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testEntry();



};

#endif // UT_APPDATA_H
