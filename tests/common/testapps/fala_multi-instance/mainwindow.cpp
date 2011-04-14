/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of libmeegotouch.
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

#include <iostream>

#include "mainwindow.h"

#include <QFile>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void FANGORNLOG(const char* s)
{
    QFile f("/tmp/fala_multi-instance.log");
    f.open(QIODevice::Append);
    f.write(s, qstrlen(s));
    f.close();
}

void timestamp(const char *s)
{
    timeval tim;
    char msg[80];
    gettimeofday(&tim, NULL);
    snprintf(msg, 80, "%d%03d %s\n", 
             static_cast<int>(tim.tv_sec), static_cast<int>(tim.tv_usec/1000), s);
    FANGORNLOG(msg);
}

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
}

bool MainWindow::event(QEvent *event)
{
    bool retVal = MApplicationWindow::event(event);
    if (event->type() == QEvent::WindowStateChange)
    {
        char message[32];
        if (isMinimized()) 
        {
            std::cerr << "Minimized" << std::endl;
            snprintf(message, 32, "%i Minimized", getpid());
            timestamp(message);
        }
        else
        {
            std::cerr << "Maximized"  << std::endl;
            snprintf(message, 32, "%i Maximized", getpid());
            timestamp(message);
        }
    }
    return retVal;
}

