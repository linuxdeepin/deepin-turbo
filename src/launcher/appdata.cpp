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

#include "appdata.h"

AppData::AppData() :
    m_options(0),
    m_argc(0),
    m_argv(NULL),
    m_appName(""),
    m_fileName(""),
    m_prio(0),
    m_entry(NULL),
    m_ioDescriptors()
{}

void AppData::setOptions(int newOptions)
{
    m_options = newOptions;
}

int AppData::options() const
{
    return m_options;
}

void AppData::setArgc(int newArgc)
{
    m_argc = newArgc;
}

int AppData::argc() const
{
    return m_argc;
}

void AppData::setArgv(const char ** newArgv)
{
    m_argv = newArgv;
}

const char ** AppData::argv() const
{
    return m_argv;
}

void AppData::setAppName(const string & newAppName)
{
    m_appName = newAppName;
}

const string & AppData::appName() const
{
    return m_appName;
}

void AppData::setFileName(const string & newFileName)
{
    m_fileName = newFileName;
}

const string & AppData::fileName() const
{
    return m_fileName;
}

void AppData::setPriority(int newPriority)
{
    m_prio = newPriority;
}

int AppData::priority() const
{
    return m_prio;
}

void AppData::setEntry(entry_t newEntry)
{
    m_entry = newEntry;
}

entry_t AppData::entry() const
{
    return m_entry;
}

const vector<int> & AppData::ioDescriptors() const
{
    return m_ioDescriptors;
}

void AppData::setIODescriptors(const vector<int> & newIODescriptors)
{
    m_ioDescriptors = newIODescriptors;
}

void AppData::deleteArgv()
{
    if (m_argv)
    {
        for (int i = 0; i < m_argc; i++)
        {
            delete m_argv[i];
            m_argv[i] = NULL;
        }

        delete [] m_argv;
        m_argv = NULL;
    }
}

AppData::~AppData()
{
    deleteArgv();
}
