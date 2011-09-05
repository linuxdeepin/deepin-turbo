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

#ifndef UT_LOGGER_H
#define UT_LOGGER_H

#include <QtTest/QtTest>
#include <QObject>
#include <tr1/memory>

#define UNIT_TEST

class Logger;

class Ut_Logger : public QObject
{
    Q_OBJECT

public:
    Ut_Logger();
    virtual ~Ut_Logger();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testOpenLog();
    void testSetDebugMode();
    void testLogDebug();
    void testLogError();
    void testLogInfo();
    void testLogWarning();
    void testCloseLog();
    void testOpenLogTwice();

private:
    char m_testPhrase[255];
    bool _findPhraseInSyslog();
    void _setTestPhrase(const char * base);
    std::tr1::shared_ptr<Logger> m_subject;

};

#endif // UT_LOGGER_H
