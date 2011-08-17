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

#ifndef MULTIWINDOWCONTENT_H
#define MULTIWINDOWCONTENT_H

#include <MApplicationWindow>

#include "mainpage.h"

class MultiWindowContent
{
public:
    static const int NUM_WINDOWS = 3;

    MultiWindowContent(bool windowFromCache);
    virtual ~MultiWindowContent();
    void activateWindow(int index);

    void createWindows();

private:
    MApplicationWindow *m_window[NUM_WINDOWS];
    MainPage *m_mainPage[NUM_WINDOWS];
    bool m_windowFromCache;
};

#endif // MULTIWINDOWCONTENT_H
