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

#include <QString>
#include <QProcess>
#include <QDateTime>
#include <QtGlobal>
#include <sys/syslog.h>

#include "ut_logger.h"
#include "logger.h"

static const QString syslogFile = "/var/log/syslog";
static const char * progName = "Ut_Logger";
static const int delay = 1000;

Ut_Logger::Ut_Logger() :
    m_subject(new Logger)
{}

Ut_Logger::~Ut_Logger()
{}

void Ut_Logger::initTestCase()
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
}

void Ut_Logger::cleanupTestCase()
{}

void Ut_Logger::testOpenLog() {
    QVERIFY(m_subject->m_isOpened == false);
    Logger::openLog(progName);
    QVERIFY(m_subject->m_isOpened == true);
}

void Ut_Logger::testSetDebugMode() {
    QVERIFY(m_subject->m_debugMode == false);
    Logger::setDebugMode(true);
    QVERIFY(m_subject->m_debugMode == true);
}


void Ut_Logger::testLogDebug() {
    //case with debug mode on
    Logger::setDebugMode(true);
    _setTestPhrase(Q_FUNC_INFO);
    Logger::logDebug(m_testPhrase);
    QTest::qSleep(delay);
    QVERIFY(_findPhraseInSyslog() == true);

    //case with debug mode off
    Logger::setDebugMode(false);
    _setTestPhrase(Q_FUNC_INFO);
    Logger::logDebug(m_testPhrase);
    QTest::qSleep(delay);
    QVERIFY(_findPhraseInSyslog() == false);
}

void Ut_Logger::testLogError() {
    _setTestPhrase(Q_FUNC_INFO);
    Logger::logError(m_testPhrase);
    QTest::qSleep(delay);
    QVERIFY(_findPhraseInSyslog() == true);
}

void Ut_Logger::testLogInfo() {
    _setTestPhrase(Q_FUNC_INFO);
    Logger::logInfo(m_testPhrase);
    QTest::qSleep(delay);
    QVERIFY(_findPhraseInSyslog() == true);
}

void Ut_Logger::testLogWarning() {
    _setTestPhrase(Q_FUNC_INFO);
    Logger::logWarning(m_testPhrase);
    QTest::qSleep(delay);
    QVERIFY(_findPhraseInSyslog() == true);
}

void Ut_Logger::testCloseLog() {
    QVERIFY(m_subject->m_isOpened == true);
    Logger::closeLog();
    QVERIFY(m_subject->m_isOpened == false);
}

void Ut_Logger::testOpenLogTwice() {
    //open log first time with random name
    _setTestPhrase(Q_FUNC_INFO);
    Logger::openLog(m_testPhrase);
    QVERIFY(m_subject->m_isOpened == true);
    Logger::logInfo("Testing OpenLog twice");
    QVERIFY(_findPhraseInSyslog() == true);
    //open log second time with another random name
    _setTestPhrase(Q_FUNC_INFO);
    Logger::openLog(m_testPhrase);
    QVERIFY(m_subject->m_isOpened == true);
    Logger::logInfo("Testing OpenLog twice");
    QVERIFY(_findPhraseInSyslog() == true);
    Logger::closeLog();
}


bool Ut_Logger::_findPhraseInSyslog()
{
    QTest::qSleep(delay);
    int exitCode = QProcess::execute("grep", QStringList() << QString(m_testPhrase) << syslogFile);
    return exitCode == 0;
}

void Ut_Logger::_setTestPhrase(const char * base)
{
    sprintf(m_testPhrase, "%s: %d", base, qrand());
}

QTEST_APPLESS_MAIN(Ut_Logger)
