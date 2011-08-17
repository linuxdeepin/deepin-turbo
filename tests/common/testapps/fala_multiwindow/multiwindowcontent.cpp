/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applifed.
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

#include "multiwindowcontent.h"
#include "mainpage.h"

#include <MDebug>
#include <MApplicationWindow>

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif


MultiWindowContent::MultiWindowContent(bool windowFromCache):
    m_windowFromCache(windowFromCache)
{
}

void MultiWindowContent::createWindows()
{
    // Create windows
    for (int i = 0; i < NUM_WINDOWS; i++) {
#ifdef HAVE_MCOMPONENTCACHE
        m_window[i] = m_windowFromCache? MComponentCache::mApplicationWindow() : new MApplicationWindow;
#else
        m_window[i] = new MApplicationWindow;
#endif
        m_window[i]->setWindowTitle(QString("Window %1").arg(i + 1));
        m_window[i]->setObjectName(QString("Window %1").arg(i + 1));
    }

    // Create pages
    for (int i = 0; i < NUM_WINDOWS; i++)
        m_mainPage[i] = new MainPage(QString("Window %1").arg(i + 1));
}

void MultiWindowContent::activateWindow(int index)
{
    index--;
    if (index >= 0 && index < NUM_WINDOWS)  {

        // Show the desired window
        m_window[index]->show();
        m_window[index]->activateWindow();
        m_window[index]->raise();

        m_mainPage[index]->appear(m_window[index]);
    }
}


MultiWindowContent::~MultiWindowContent()
{
}

